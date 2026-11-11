// Offline contract test using the original EX603 cipher and real directional keys.
// Usage: packet_crypto_test <MuServer/Data/Hack> <MuClientEX603/Data>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../../GameServer/stdafx.h"
#include "../../GameServer/PacketManager.h"
#include "../../GameServer/HackCheck.h"
#include "../../GameServer/ServerInfo.h"
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace
{
	using Bytes = std::vector<BYTE>;
	void require(bool condition,const char* message)
	{
		if(!condition) throw std::runtime_error(message);
	}
	Bytes read(const std::filesystem::path& path)
	{
		std::ifstream stream(path,std::ios::binary);
		if(!stream) throw std::runtime_error("Required real cipher asset is missing: "+path.string());
		return Bytes(std::istreambuf_iterator<char>(stream),std::istreambuf_iterator<char>());
	}
	bool load(CPacketManager& manager,const std::filesystem::path& path,bool decrypt)
	{
		auto text = path.string();
		return decrypt ? manager.LoadDecryptionKey(text.data()) : manager.LoadEncryptionKey(text.data());
	}
	struct TemporaryDirectory
	{
		std::filesystem::path path;
		TemporaryDirectory()
		{
			std::string pattern = (std::filesystem::temp_directory_path()/"mu-ex603-crypto-XXXXXX").string();
			char* made = ::mkdtemp(pattern.data());
			if(made == nullptr) throw std::runtime_error("Cannot create offline test temporary directory");
			path = made;
		}
		~TemporaryDirectory() { std::error_code error; std::filesystem::remove_all(path,error); }
	};
	void write(const std::filesystem::path& path,const Bytes& bytes)
	{
		std::ofstream stream(path,std::ios::binary|std::ios::trunc);
		stream.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
		require(static_cast<bool>(stream),"Cannot write offline key test fixture");
	}
	Bytes encrypt(CPacketManager& manager,const Bytes& plaintext)
	{
		// Original EncryptBlock examines eight bytes even for a shorter last block.
		Bytes padded(((plaintext.size()+7)/8)*8,0);
		std::copy(plaintext.begin(),plaintext.end(),padded.begin());
		const int expected = static_cast<int>(((plaintext.size()+7)/8)*11);
		Bytes ciphertext(expected,0);
		require(manager.Encrypt(ciphertext.data(),padded.data(),static_cast<int>(plaintext.size())) == expected,"Cipher expansion is not 8 to 11 bytes");
		return ciphertext;
	}
	Bytes decryptChecked(CPacketManager& manager,const Bytes& ciphertext)
	{
		require(!ciphertext.empty() && ciphertext.size()%11 == 0,"Encrypted block framing is invalid");
		Bytes plaintext((ciphertext.size()/11)*8,0);
		std::size_t length = 0;
		for(std::size_t offset = 0; offset < ciphertext.size(); offset += 11)
		{
			std::array<BYTE,8> block{};
			const int size = manager.DecryptBlock(block.data(),const_cast<BYTE*>(ciphertext.data()+offset));
			require(size >= 1 && size <= 8,"Cipher checksum or block length failed");
			require(offset+11 == ciphertext.size() || size == 8,"A nonfinal cipher block is short");
			std::copy(block.begin(),block.begin()+size,plaintext.begin()+length);
			length += size;
		}
		plaintext.resize(length);
		return plaintext;
	}
	void cipherDirection(CPacketManager& encryptor,CPacketManager& decryptor)
	{
		for(int length = 1; length <= 513; ++length)
		{
			Bytes plaintext(length);
			for(int index = 0; index < length; ++index) plaintext[index] = static_cast<BYTE>((index*73+length*19)&255);
			const auto ciphertext = encrypt(encryptor,plaintext);
			require(decryptChecked(decryptor,ciphertext) == plaintext,"Real directional keys do not roundtrip");
			Bytes decoded(((ciphertext.size()/11)*8)+1,0);
			// Frame normalization writes plaintext at offset one. This intentionally
			// exercises alignment on native ARM instead of assuming WORD alignment.
			const int count = decryptor.Decrypt(decoded.data()+1,const_cast<BYTE*>(ciphertext.data()),static_cast<int>(ciphertext.size()));
			require(count == length && std::equal(plaintext.begin(),plaintext.end(),decoded.begin()+1),"Original full cipher primitive disagrees with checked blocks");
		}
		const auto ciphertext = encrypt(encryptor,Bytes(17,0x5A));
		auto corrupt = ciphertext;
		corrupt[corrupt.size()-1] ^= 1; // Last block checksum, after two valid blocks.
		std::array<BYTE,8> block{};
		require(decryptor.DecryptBlock(block.data(),corrupt.data()+corrupt.size()-11) == -1,"A corrupt later cipher block was accepted");
		Bytes fullOutput(24,0);
		require(decryptor.Decrypt(fullOutput.data(),corrupt.data(),static_cast<int>(corrupt.size())) == -1,"Full decrypt ignored a corrupt later block");
		require(decryptor.Decrypt(block.data(),corrupt.data(),1) == -1,"Partial encrypted block was accepted");
	}
	void keyValidation(const std::filesystem::path& realKey)
	{
		const auto real = read(realKey);
		require(real.size() == 54,"Real key file must contain 54 bytes");
		CPacketManager manager;
		require(load(manager,realKey,false),"Valid real encryption key was rejected");
		TemporaryDirectory directory;
		const auto fixture = directory.path/"invalid.dat";
		for(std::size_t length = 0; length < real.size(); ++length)
		{
			write(fixture,Bytes(real.begin(),real.begin()+length));
			require(!load(manager,fixture,false),"Truncated key file was accepted");
		}
		auto invalid = real;
		invalid.push_back(0);
		write(fixture,invalid);
		require(!load(manager,fixture,false),"Trailing key bytes were accepted");
		invalid = real; invalid[0] ^= 1; write(fixture,invalid);
		require(!load(manager,fixture,false),"Incorrect key file header was accepted");
		invalid = real; invalid[2] ^= 1; write(fixture,invalid);
		require(!load(manager,fixture,false),"Incorrect encoded file size was accepted");
		// XOR-obfuscated zero modulus/key using the source's first load mask.
		constexpr std::array<BYTE,4> zeroEncoded{{0x9B,0xA7,0x08,0x3F}};
		for(const std::size_t offset : {std::size_t(6),std::size_t(22)})
		{
			invalid = real;
			std::copy(zeroEncoded.begin(),zeroEncoded.end(),invalid.begin()+offset);
			write(fixture,invalid);
			require(!load(manager,fixture,false),"Zero modulus/key was accepted");
		}
		require(!load(manager,directory.path/"missing.dat",false),"Missing key was accepted");
		std::array<BYTE,8> input{};
		std::array<BYTE,11> output{};
		require(manager.EncryptBlock(output.data(),input.data(),8) == -1,"Failed key reload left the cipher usable");
		require(load(manager,realKey,false),"A valid key could not be reloaded after failures");
	}
	constexpr std::array<BYTE,32> xorTable{{
		0xAB,0x11,0xCD,0xFE,0x18,0x23,0xC5,0xA3,0xCA,0x33,0xC1,0xCC,0x66,0x67,0x21,0xF3,
		0x32,0x12,0x15,0x35,0x29,0xFF,0xFE,0x1D,0x44,0xEF,0xCD,0x41,0x26,0x3C,0x4E,0x4D}};
	Bytes inboundXor(Bytes packet)
	{
		const std::size_t headOffset = packet[0] == 0xC1 ? 2 : 3;
		for(std::size_t index = headOffset+1; index < packet.size(); ++index)
			packet[index] ^= packet[index-1]^xorTable[index%32];
		return packet;
	}
	void frames(CPacketManager& client,CPacketManager& server)
	{
		for(const bool wide : {false,true})
		{
			Bytes clear(wide ? 301 : 37);
			for(std::size_t n = 0; n < clear.size(); ++n) clear[n] = static_cast<BYTE>(n*23);
			clear[0] = wide ? 0xC2 : 0xC1;
			if(wide) { clear[1] = HIBYTE(clear.size()); clear[2] = LOBYTE(clear.size()); clear[3] = 0xF3; }
			else { clear[1] = static_cast<BYTE>(clear.size()); clear[2] = 0xF1; }
			const auto xored = inboundXor(clear);
			Bytes result(clear.size());
			require(server.AddData(const_cast<BYTE*>(xored.data()),static_cast<int>(xored.size())) && server.ExtractPacket(result.data()),"C1/C2 input frame was rejected");
			require(result == clear,"C1/C2 chained XOR failed to recover source frame");
			const std::size_t serialOffset = wide ? 2 : 1;
			Bytes plaintext(xored.begin()+serialOffset,xored.end());
			plaintext[0] = 0xA7;
			const auto ciphertext = encrypt(client,plaintext);
			Bytes wire(wide ? 3 : 2);
			wire[0] = wide ? 0xC4 : 0xC3;
			wire.insert(wire.end(),ciphertext.begin(),ciphertext.end());
			if(wide) { wire[1] = HIBYTE(wire.size()); wire[2] = LOBYTE(wire.size()); }
			else { require(wire.size() <= 255,"C3 fixture exceeds one-byte size"); wire[1] = static_cast<BYTE>(wire.size()); }
			const auto originalWire = wire;
			EncryptData(wire.data(),static_cast<int>(wire.size()));
			// Transform receive chunks separately: the outer transform is bytewise.
			DecryptData(wire.data(),1);
			DecryptData(wire.data()+1,static_cast<int>(wire.size()-1));
			require(wire == originalWire,"Outer client transform depends on recv chunking");
			const Bytes decoded = decryptChecked(server,Bytes(wire.begin()+(wide ? 3 : 2),wire.end()));
			require(decoded[0] == 0xA7,"C3/C4 serial was not recovered");
			Bytes normalized(serialOffset);
			normalized.insert(normalized.end(),decoded.begin(),decoded.end());
			normalized[0] = wide ? 0xC2 : 0xC1;
			if(wide) { normalized[1] = HIBYTE(normalized.size()); normalized[2] = LOBYTE(normalized.size()); }
			else normalized[1] = static_cast<BYTE>(normalized.size());
			require(server.AddData(normalized.data(),static_cast<int>(normalized.size())) && server.ExtractPacket(result.data()),"Normalized C3/C4 frame was rejected");
			require(result == clear,"C3/C4 decrypt/serial/frame/XOR pipeline changed the payload");
		}
		Bytes oversized(2048,0);
		require(!server.AddData(oversized.data(),static_cast<int>(oversized.size())),"Source 2048-byte PacketManager bound changed");
		Bytes truncated{{0xC2,0,8,0xF3,0}};
		std::array<BYTE,16> output{};
		require(server.AddData(truncated.data(),static_cast<int>(truncated.size())) && !server.ExtractPacket(output.data()),"Incomplete C2 frame was accepted");
	}
	void outerTransforms()
	{
		for(const bool secondary : {false,true})
		{
			std::memset(CustomerName,0,32);
			std::memcpy(gServerInfo.m_ServerSerial,"TESTEX603SERIAL1",16);
			gServerInfo.m_ServerSerial[16] = 0;
			gServerInfo.m_ServerEncDecKey1 = secondary ? 5 : 0;
			gServerInfo.m_ServerEncDecKey2 = secondary ? 7 : 0;
			InitHackCheck();
			Bytes plain(512);
			for(std::size_t n = 0; n < plain.size(); ++n) plain[n] = static_cast<BYTE>(n);
			auto encoded = plain;
			EncryptData(encoded.data(),static_cast<int>(encoded.size()));
			require(encoded != plain,"Configured outer transform unexpectedly does nothing");
			for(std::size_t n = 0; n < encoded.size(); ++n) DecryptData(encoded.data()+n,1);
			require(encoded == plain,"Outer transform failed bytewise inversion");
		}
	}
}

int main(int argc,char** argv)
{
	try
	{
		if(argc != 3) throw std::runtime_error("Usage: packet_crypto_test <MuServer/Data/Hack> <MuClientEX603/Data> (real assets required)");
		const std::filesystem::path serverDirectory(argv[1]),clientDirectory(argv[2]);
		CPacketManager server,client;
		require(load(server,serverDirectory/"Enc2.dat",false) && load(server,serverDirectory/"Dec1.dat",true),"Server directional keys failed to load");
		require(load(client,clientDirectory/"Enc1.dat",false) && load(client,clientDirectory/"Dec2.dat",true),"Client directional keys failed to load");
		keyValidation(serverDirectory/"Enc2.dat");
		cipherDirection(client,server);
		cipherDirection(server,client);
		outerTransforms();
		frames(client,server);
		std::cout<<"EX603 real directional cipher, packed key validation, outer transforms and C1-C4 frame contracts passed; no sockets opened.\n";
		return 0;
	}
	catch(const std::exception& error)
	{
		std::cerr<<"packet_crypto_test: "<<error.what()<<'\n';
		return 1;
	}
}
