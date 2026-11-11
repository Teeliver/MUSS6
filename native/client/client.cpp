#include "client.hpp"
#include "evil_spirit.hpp"
#include <thread>
#include "stdafx.h"
#include "Protocol.h"
#include "PacketManager.h"
#include "HidManager.h"
#include "ItemManager.h"
#include "NpcTalk.h"
#include "QuestWorld.h"
#include "Shop.h"
#include "SkillManager.h"
#include "Warehouse.h"
#include <CommonCrypto/CommonDigest.h>
#include <uuid/uuid.h>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <cstring>
#include <memory>
#include <set>
#include <stdexcept>
#include <optional>
namespace mu::client {
static_assert(sizeof(PMSG_CONNECT_ACCOUNT_RECV)==59 && offsetof(PMSG_CONNECT_ACCOUNT_RECV,TickCount)==34,"EX603 login wire layout changed");
static_assert(sizeof(PMSG_CHARACTER_LIST_SEND)==8 && sizeof(PMSG_CHARACTER_LIST)==34,"EX603 character-list wire layout changed");
static_assert(sizeof(PMSG_CHARACTER_INFO_SEND)==136,"EX603 EXTRA1 world-entry layout changed");
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
unsigned le16(const Bytes& b,std::size_t i){return b.at(i)|(unsigned(b.at(i+1))<<8);}
std::string setting(const std::string& file,const char* section,const char* key){
 char value[4096]{};GetPrivateProfileString(section,key,"",value,sizeof(value),file.c_str());return value;
}
class Socket {
 int fd=-1;Bytes pending;std::size_t consumed=0;bool eof=false;std::chrono::steady_clock::time_point partialSince{};
 void wait(short flags){
  pollfd p{fd,flags,0};int result;
  do{result=poll(&p,1,3000);}while(result<0&&errno==EINTR);
  require(result>0,"Network timeout or poll failure");
  require((p.revents&flags)!=0,"Peer disconnected before the complete packet arrived");
 }
public:
 Socket(const std::string& host,int port){
  sockaddr_in address{};address.sin_family=AF_INET;
  require(port>0&&port<=65535,"Port must be 1..65535");
  require(inet_pton(AF_INET,host.c_str(),&address.sin_addr)==1,"An IPv4 address is required");
  address.sin_port=htons(port);
  fd=socket(AF_INET,SOCK_STREAM,0);require(fd>=0,"Could not create socket");
  try{
   require(fcntl(fd,F_SETFL,O_NONBLOCK)==0,"Could not configure nonblocking socket");
   int noDelay=1;require(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&noDelay,sizeof(noDelay))==0,"Could not disable TCP batching");
   int noSigPipe=1;require(setsockopt(fd,SOL_SOCKET,SO_NOSIGPIPE,&noSigPipe,sizeof(noSigPipe))==0,"Could not configure socket");
   if(::connect(fd,reinterpret_cast<sockaddr*>(&address),sizeof(address))!=0){
    require(errno==EINPROGRESS,"Connection refused or endpoint unavailable");wait(POLLOUT);
    int error=0;socklen_t length=sizeof(error);
    require(getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&length)==0&&error==0,"Connection refused or endpoint unavailable");
   }
  }catch(...){close(fd);fd=-1;throw;}
 }
 ~Socket(){if(fd>=0)close(fd);}
 Socket(const Socket&)=delete;Socket& operator=(const Socket&)=delete;
 Bytes read(std::size_t count,const Profile* profile=nullptr){
  Bytes b(count);std::size_t n=0;
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(n<count){
   require(std::chrono::steady_clock::now()<deadline,"Packet read deadline exceeded");
   auto result=recv(fd,b.data()+n,count-n,0);
   if(result>0)n+=static_cast<std::size_t>(result);
   else if(result<0&&(errno==EAGAIN||errno==EWOULDBLOCK))wait(POLLIN);
   else if(result<0&&errno==EINTR)continue;
   else throw std::runtime_error("Peer closed connection before complete packet");
  }
  if(profile)decode_transport(b,*profile);return b;
 }
 std::optional<Bytes> try_frame(const Profile* profile=nullptr){
  if(consumed>0&&(consumed>=pending.size()/2)){pending.erase(pending.begin(),pending.begin()+consumed);consumed=0;}
  auto extract=[&]() -> std::optional<Bytes>{
   auto left=pending.size()-consumed;
   if(left>0){unsigned type=pending[consumed];require(type>=0xC1&&type<=0xC4,"Unexpected frame type");bool wide=type==0xC2||type==0xC4;unsigned header=wide?3:2;
    if(left>=header){unsigned size=wide?(unsigned(pending[consumed+1])<<8)|pending[consumed+2]:pending[consumed+1];require(size>=header+1&&size<=16384,"Invalid inbound frame length");if(left>=size){Bytes frame(pending.begin()+consumed,pending.begin()+consumed+size);consumed+=size;partialSince={};return frame;}}
    auto now=std::chrono::steady_clock::now();if(partialSince==std::chrono::steady_clock::time_point{})partialSince=now;require(now-partialSince<std::chrono::seconds(5),"Incomplete packet deadline exceeded");
   }else partialSince={};return {};
  };
  if(auto ready=extract())return ready;
  std::uint8_t chunk[8192];
  for(unsigned i=0;i<8&&!eof;i++){
   auto n=recv(fd,chunk,sizeof(chunk),0);
   if(n>0){Bytes b(chunk,chunk+n);if(profile)decode_transport(b,*profile);pending.insert(pending.end(),b.begin(),b.end());require(pending.size()-consumed<=262144,"Inbound buffering limit exceeded");if(auto ready=extract())return ready;}
   else if(n==0){eof=true;break;}
   else if(errno==EINTR)continue;else if(errno==EAGAIN||errno==EWOULDBLOCK)break;else throw std::runtime_error("Socket receive failed");
  }
  require(!eof,"Peer closed the session");return {};
 }
 Bytes frame(const Profile* profile=nullptr){
  auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  for(;;){auto packet=try_frame(profile);if(packet)return std::move(*packet);require(std::chrono::steady_clock::now()<deadline,"Packet receive deadline exceeded");wait(POLLIN);}
 }
 void send(const Bytes& b){
  std::size_t n=0;
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
  while(n<b.size()){
   require(std::chrono::steady_clock::now()<deadline,"Packet send deadline exceeded");
   auto result=::send(fd,b.data()+n,b.size()-n,0);
   if(result>0)n+=static_cast<std::size_t>(result);
   else if(result<0&&(errno==EAGAIN||errno==EWOULDBLOCK))wait(POLLOUT);
   else if(result<0&&errno==EINTR)continue;
   else throw std::runtime_error("Could not send packet");
  }
 }
};
Bytes response(Socket& peer,unsigned type,unsigned head,unsigned sub){
 for(int i=0;i<8;i++){
  auto b=peer.frame();auto offset=b[0]==0xC2?3u:2u;
  if(b[0]==type&&b.size()>offset+1&&b[offset]==head&&b[offset+1]==sub)return b;
 }
 throw std::runtime_error("Expected protocol response was not received");
}
void handshake(Socket& s){require(s.frame()==Bytes({0xC1,4,0,1}),"ConnectServer rejected the connection");}
}
Profile load_profile(const std::string& path){
 const std::string file=path.empty()?MU_CLIENT_DEFAULT_CONFIG:path;
 require(std::filesystem::is_regular_file(file),"Client profile file not found");
 Profile p;p.connectHost=setting(file,"Client","ConnectHost");
 p.connectPort=GetPrivateProfileInt("Client","ConnectPort",44405,file.c_str());
 p.assetDirectory=setting(file,"Client","AssetDirectory");
 auto pace=setting(file,"Client","MovementRate");if(!pace.empty()){std::size_t used=0;p.movementRate=std::stod(pace,&used);require(used==pace.size()&&p.movementRate>=.5&&p.movementRate<=1.25,"MovementRate must be 0.5..1.25");}
 auto language=setting(file,"Client","Language");if(!language.empty())p.language=language;
 auto runtime=setting(file,"Client","GameRuntime");
 auto common=(std::filesystem::path(runtime)/"Data/GameServerInfo - Common.dat").string();
 auto serial=setting(file,"Client","ClientSerial");
 auto version=setting(file,"Client","ClientVersion");
 bool standalone=!serial.empty()&&!version.empty();
 if(!standalone){
  require(std::filesystem::is_regular_file(common),"ClientSerial/ClientVersion or GameRuntime config is required");
  serial=setting(common,"GameServerInfo","ServerSerial");
  version=setting(common,"GameServerInfo","ServerVersion");
 }
 require(serial.size()==16,"Client serial must have 16 bytes");p.serial=serial;
 for(char c:version)if(c!='.')p.version+=c;
 // Main_EX603 uses ClientSerial[17]: 16 serial bytes plus the terminating NUL.
 std::uint16_t key=0,sum=0;const std::string customer="MuOnline";
 for(unsigned n=0;n<32;n++){
  auto c=n<customer.size()?std::uint8_t(customer[n]):std::uint8_t(0);
  auto s=n%17<serial.size()?std::uint8_t(serial[n%17]):std::uint8_t(0);
  key=std::uint16_t(key+std::uint8_t(c^s));key^=std::uint8_t(c-s);sum=std::uint16_t(sum+c);
 }
 p.key1=std::uint8_t(0xAB+(key&255));p.key2=std::uint8_t(0xCD+(key>>8));
 int a=standalone?GetPrivateProfileInt("Client","ServerEncDecKey1",0,file.c_str()):GetPrivateProfileInt("GameServerInfo","ServerEncDecKey1",0,common.c_str());
 int b=standalone?GetPrivateProfileInt("Client","ServerEncDecKey2",0,file.c_str()):GetPrivateProfileInt("GameServerInfo","ServerEncDecKey2",0,common.c_str());
 require(a>=0&&a<=255&&b>=0&&b<=255,"MHP key config must be 0..255");
 p.mhp=a!=0||b!=0;p.mhp1=std::uint8_t(a+(sum&255));p.mhp2=std::uint8_t(b+(sum>>8));
 return p;
}
void decode_transport(Bytes& b,const Profile& p){
 for(auto& value:b){
  if(p.mhp)value=std::uint8_t((value^p.mhp1)-p.mhp2);
  value=std::uint8_t((value^p.key1)-(unsigned(p.key1)*p.key2));
 }
}
std::vector<Server> parse_list(const Bytes& b){
 require(b.size()>=7&&b[0]==0xC2&&((unsigned(b[1])<<8)|b[2])==b.size()&&b[3]==0xF4&&b[4]==6,"Invalid server-list response");
 unsigned count=(unsigned(b[5])<<8)|b[6];require(b.size()==7+count*4,"Server-list count/length mismatch");
 std::vector<Server> list;std::set<unsigned> codes;
 for(unsigned i=0;i<count;i++){
  unsigned n=7+i*4,code=le16(b,n);require(codes.insert(code).second,"Duplicate server code");
  list.push_back({code,b[n+2],b[n+3]});
 }
 return list;
}
Greeting parse_greeting(const Bytes& b){
 require(b.size()==14&&b[0]==0xC1&&b[1]==14&&b[2]==0xF1&&b[3]==0,"Invalid EX603 EXTRA1 GameServer handshake");
 require(b[4]==1,"GameServer rejected the connection");
 Greeting g{(unsigned(b[5])<<8)|b[6],le16(b,12),""};
 // Return encoded version bytes, without inventing an application-level version match.
 for(unsigned i=7;i<12;i++){if(i>7)g.version+=' ';g.version+=std::to_string(b[i]);}
 return g;
}
std::vector<Server> discover(const Profile& p){
 Socket peer(p.connectHost,p.connectPort);handshake(peer);peer.send({0xC1,4,0xF4,6});return parse_list(response(peer,0xC2,0xF4,6));
}
std::string connect_game(const Profile& p,unsigned code){
 require(code<=255,"EX603 ConnectServer F4:03 only supports one-byte server codes");
 Socket cs(p.connectHost,p.connectPort);handshake(cs);cs.send({0xC1,5,0xF4,3,std::uint8_t(code)});
 auto endpoint=response(cs,0xC1,0xF4,3);require(endpoint.size()==22,"Invalid GameServer endpoint response");
 auto first=endpoint.begin()+4,last=endpoint.begin()+20;auto nul=std::find(first,last,0);
 require(nul!=last,"Server address is not NUL terminated");std::string host(first,nul);unsigned port=le16(endpoint,20);
 Socket game(host,port);auto g=parse_greeting(game.frame(&p));require(g.serverCode==code,"GameServer code does not match selection");
 return "PASS: GameServer "+host+":"+std::to_string(port)+"; F1:00 accepted; index="+std::to_string(g.index)+"; encoded version=["+g.version+"].\nConnection closed cleanly. Login and gameplay have not been tested.";
}
void encode_transport(Bytes& b,const Profile& p){
 for(auto& v:b){v=std::uint8_t((v+unsigned(p.key1)*p.key2)^p.key1);if(p.mhp)v=std::uint8_t((v+p.mhp2)^p.mhp1);}
}
namespace {
constexpr std::uint8_t xorFilter[32]={0xAB,0x11,0xCD,0xFE,0x18,0x23,0xC5,0xA3,0xCA,0x33,0xC1,0xCC,0x66,0x67,0x21,0xF3,0x32,0x12,0x15,0x35,0x29,0xFF,0xFE,0x1D,0x44,0xEF,0xCD,0x41,0x26,0x3C,0x4E,0x4D};
void name_check(const std::string& s){require(!s.empty()&&s.size()<=10,"Name must contain 1..10 ASCII letters or digits");for(unsigned char c:s)require((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9'),"Name must contain ASCII letters or digits");}
template<class T> Bytes bytes(const T& t){auto p=reinterpret_cast<const std::uint8_t*>(&t);return Bytes(p,p+sizeof(t));}
std::string fixed_name(const char* p,std::size_t n){return std::string(p,std::find(p,p+n,'\0'));}
}
Bytes login_packet(const Profile& p,const std::string& account,const std::string& password,std::uint32_t tick){
 name_check(account);require(!password.empty()&&password.size()<=10,"EX603 account password must contain 1..10 ASCII bytes");
 for(unsigned char c:password)require(c>=33&&c<=126,"Password must contain printable ASCII bytes without spaces");
 require(p.serial.size()==16&&p.version.size()==5,"Invalid EX603 version or serial");
 PMSG_CONNECT_ACCOUNT_RECV packet{};packet.header.setE(0xF1,1,sizeof(packet));
 constexpr unsigned char keys[3]={0xFC,0xCF,0xAB};
 for(unsigned i=0;i<10;i++){packet.account[i]=char((i<account.size()?static_cast<unsigned char>(account[i]):0)^keys[i%3]);packet.password[i]=char((i<password.size()?static_cast<unsigned char>(password[i]):0)^keys[i%3]);}
 packet.TickCount=tick;std::memcpy(packet.ClientVersion,p.version.data(),5);std::memcpy(packet.ClientSerial,p.serial.data(),16);
 return bytes(packet);
}
CharacterList parse_characters(const Bytes& b){
 require(b.size()>=sizeof(PMSG_CHARACTER_LIST_SEND)&&b[0]==0xC1&&b[1]==b.size()&&b[2]==0xF3&&b[3]==0,"Invalid character-list packet");
 PMSG_CHARACTER_LIST_SEND head{};std::memcpy(&head,b.data(),sizeof(head));
 require(head.count<=5&&b.size()==sizeof(head)+head.count*sizeof(PMSG_CHARACTER_LIST),"Character-list count/length mismatch");
 CharacterList list{head.ClassCode,head.ExtWarehouse,{}};std::set<unsigned> slots;std::set<std::string> names;
 for(unsigned i=0;i<head.count;i++){
  PMSG_CHARACTER_LIST row{};std::memcpy(&row,b.data()+sizeof(head)+i*sizeof(row),sizeof(row));
  auto name=fixed_name(row.Name,10);name_check(name);
  require(row.slot<5&&slots.insert(row.slot).second&&names.insert(name).second,"Invalid or duplicate character slot/name");
  list.characters.push_back({row.slot,row.Level,row.CtlCode,name,Bytes(row.CharSet,row.CharSet+18),row.GuildStatus});
 }
 return list;
}
std::string login_result(unsigned result){
 switch(result){case 0:return "Sai mật khẩu";case 1:return "Đăng nhập thành công";case 2:return "Tài khoản không tồn tại";case 3:return "Tài khoản đang online";case 4:return "Server đầy hoặc chưa sẵn sàng";case 5:return "Tài khoản bị khóa";case 6:return "Version hoặc serial không khớp";case 8:return "Sai thông tin đăng nhập quá nhiều lần";case 9:return "Account chưa đủ cấp để vào server";default:return "Đăng nhập bị từ chối (code "+std::to_string(result)+")";}
}
int parse_boots_level(const Bytes& b){
 require(b.size()>=sizeof(PMSG_ITEM_LIST_SEND)&&b[0]==0xC2&&b[3]==0xF3&&b[4]==0x10,"Invalid inventory packet");unsigned count=b[5];require(b.size()==sizeof(PMSG_ITEM_LIST_SEND)+count*sizeof(PMSG_ITEM_LIST),"Inventory count/size mismatch");std::set<unsigned> slots;int boots=-1;
 for(unsigned i=0;i<count;i++){unsigned offset=sizeof(PMSG_ITEM_LIST_SEND)+i*sizeof(PMSG_ITEM_LIST),slot=b[offset];require(slot<INVENTORY_SIZE&&slots.insert(slot).second,"Invalid or duplicate inventory slot");if(slot==INVENTORY_SLOT_BOOTS){require((b[offset+6]>>4)==11,"Non-boot item in equipped boot slot");boots=(b[offset+2]>>3)&15;}}
 return boots;
}
std::optional<ServerNotice> parse_notice(const Bytes& b){if(b.size()<3||b[0]!=0xC1||b[2]!=0x0D)return {};require(b.size()>=14&&b[1]==b.size(),"Invalid server notice length");auto end=std::find(b.begin()+13,b.end(),0);require(end!=b.end(),"Unterminated server notice");ServerNotice n;n.type=b[3];n.count=b[4];n.opacity=b[5];n.delay=b[6]|unsigned(b[7])<<8;n.color=b[8]|unsigned(b[9])<<8|unsigned(b[10])<<16|unsigned(b[11])<<24;n.speed=b[12];n.message.assign(b.begin()+13,end);return n;}
struct Session::Impl {
 std::optional<Item> pendingPickup;std::unique_ptr<EvilSpirits> spirits;std::chrono::steady_clock::time_point spiritTime;unsigned spiritHitFrame=0;std::uint8_t skillSerial=0;
 std::function<void(const ServerNotice&)> noticeHandler;
 Profile profile;std::unique_ptr<Socket> socket;CPacketManager cipher;std::uint8_t sendSerial=0,recvSerial=0;bool logged=false,entered=false,leaving=false;unsigned index=0;WorldState world;ViewportSnapshot viewport;InventorySnapshot inventory;std::chrono::steady_clock::time_point heartbeat=std::chrono::steady_clock::now(),lastMove=std::chrono::steady_clock::time_point::min();
 std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
 std::uint32_t tick(){return static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count());}
 Impl(const Profile& p,unsigned code):profile(p){
  require(code<=255,"Server code must be 0..255");
  auto enc=(std::filesystem::path(p.assetDirectory)/"Data/Enc1.dat").string(),dec=(std::filesystem::path(p.assetDirectory)/"Data/Dec2.dat").string();
  require(cipher.LoadEncryptionKey(enc.data())&&cipher.LoadDecryptionKey(dec.data()),"Client EX603 cipher key assets could not be loaded");
  Socket cs(p.connectHost,p.connectPort);handshake(cs);cs.send({0xC1,5,0xF4,3,std::uint8_t(code)});
  auto ep=response(cs,0xC1,0xF4,3);require(ep.size()==22,"Invalid GameServer endpoint response");
  auto first=ep.begin()+4,last=ep.begin()+20,nul=std::find(first,last,0);require(nul!=last,"Invalid GameServer address");
  socket=std::make_unique<Socket>(std::string(first,nul),le16(ep,20));auto greeting=socket->frame(&profile);
  auto parsed=parse_greeting(greeting);index=parsed.index;require(parsed.serverCode==code,"GameServer code mismatch");
  require(profile.version.size()==5&&std::equal(profile.version.begin(),profile.version.end(),greeting.begin()+7),"GameServer greeting version mismatch");
  // Main_EX603 sends F3:09 after the greeting. Use a hash of the real macOS host UUID,
  // preserving the wire format without exposing the UUID or fabricating Windows IDs.
  uuid_t uuid{};timespec limit{1,0};require(gethostuuid(uuid,&limit)==0,"Could not obtain host ID for EX603 client verification");
  std::uint8_t digest[CC_SHA256_DIGEST_LENGTH]{};CC_SHA256(uuid,sizeof(uuid),digest);
  PMSG_HARDWAREID_INFO_RECV identity{};identity.header.set(0xF3,9,sizeof(identity));
  for(unsigned group=0;group<5;group++){if(group>0)identity.HardwareId[group*9-1]='-';for(unsigned byte=0;byte<4;byte++)std::snprintf(identity.HardwareId+group*9+byte*2,3,"%02X",digest[group*4+byte]);}
  send(bytes(identity));std::memset(uuid,0,sizeof(uuid));std::memset(digest,0,sizeof(digest));

 }
 void send(Bytes packet){
  require(socket!=nullptr,"Session is disconnected");require(packet.size()>=3&&packet.size()<2048,"Invalid outbound packet");
  bool wide=packet[0]==0xC2||packet[0]==0xC4;unsigned head=wide?3:2;
  // Main_EX603 encrypts XOR chaining forwards. Server reverses the chain.
  for(unsigned i=head+1;i<packet.size();i++)packet[i]^=packet[i-1]^xorFilter[i%32];
  if(packet[0]==0xC3||packet[0]==0xC4){
   unsigned off=wide?2:1,plainSize=packet.size()-off;Bytes plain(((plainSize+7)/8)*8,0);
   std::copy(packet.begin()+off,packet.end(),plain.begin());plain[0]=sendSerial++;
   Bytes output(head+((plainSize+7)/8)*11,0);require(wide||output.size()<=255,"Encrypted packet too large");
   require(cipher.Encrypt(output.data()+head,plain.data(),plainSize)==int(output.size()-head),"Packet encryption failed");
   output[0]=packet[0];if(wide){output[1]=output.size()>>8;output[2]=output.size();}else output[1]=output.size();
   std::fill(plain.begin(),plain.end(),0);std::fill(packet.begin(),packet.end(),0);packet=std::move(output);
  }
  encode_transport(packet,profile);socket->send(packet);std::fill(packet.begin(),packet.end(),0);
 }
 Bytes normalize(Bytes wire){
  if(wire[0]==0xC3||wire[0]==0xC4){
   bool wide=wire[0]==0xC4;unsigned off=wide?3:2;require((wire.size()-off)%11==0&&wire.size()>off,"Invalid encrypted frame length");
   Bytes plain((wire.size()-off)/11*8,0);unsigned n=0;
   for(unsigned i=off;i<wire.size();i+=11){int length=cipher.DecryptBlock(plain.data()+n,wire.data()+i);require(length>0&&length<=8&&(i+11==wire.size()||length==8),"Invalid encrypted block checksum or length");n+=length;}
   require(n>=2&&plain[0]==recvSerial++,"GameServer packet serial mismatch");
   Bytes normalized(wide?3:2,0);normalized[0]=wide?0xC2:0xC1;normalized.insert(normalized.end(),plain.begin()+1,plain.begin()+n);
   require(wide||normalized.size()<=255,"Oversized normalized packet");
   if(wide){normalized[1]=normalized.size()>>8;normalized[2]=normalized.size();}else normalized[1]=normalized.size();
   return normalized;
  }
  return wire;
 }
 void publish(){world.playerIndex=index;if(!world.inventory||world.inventory->revision!=inventory.revision){world.inventory=std::make_shared<const InventorySnapshot>(inventory);auto boots=inventory.items.find(INVENTORY_SLOT_BOOTS);world.bootsLevel=boots==inventory.items.end()?-1:int(boots->second.level);world.inventoryReady=inventory.ready;}if(!world.viewport||world.viewport->revision!=viewport.revision)world.viewport=std::make_shared<const ViewportSnapshot>(viewport);}
 void observe(const Bytes& b){
  apply_character_attributes(world.attributes,b);
  if(auto notice=parse_notice(b)){if(noticeHandler)noticeHandler(*notice);}
  apply_viewport_packet(viewport,b,index);
  if(apply_inventory_packet(inventory,b)){world.inventoryReady=inventory.ready;auto boots=inventory.items.find(INVENTORY_SLOT_BOOTS);if(boots!=inventory.items.end()){require(boots->second.index/512==11,"Non-boot item in equipped boot slot");world.bootsLevel=boots->second.level;}else world.bootsLevel=-1;}
  world.packets++;unsigned off=b[0]==0xC2?3:2;
  require(b.size()>off,"Truncated normalized packet");
  if(b[off]==0xF3&&b.size()>off+1&&(b[off+1]==0xE0||b[off+1]==0xE1)){auto& v=world.vitals;if(b[off+1]==0xE0){require(b.size()==sizeof(PMSG_NEW_CHARACTER_INFO_SEND),"Invalid HUD character info");PMSG_NEW_CHARACTER_INFO_SEND p{};std::memcpy(&p,b.data(),sizeof(p));v={p.ViewCurHP,p.ViewMaxHP,p.ViewCurMP,p.ViewMaxMP,p.ViewCurBP,p.ViewMaxBP,p.ViewCurSD,p.ViewMaxSD,p.Experience,p.NextExperience,p.Level,true};}else{require(b.size()==sizeof(PMSG_NEW_CHARACTER_CALC_SEND),"Invalid HUD character calculation");PMSG_NEW_CHARACTER_CALC_SEND p{};std::memcpy(&p,b.data(),sizeof(p));v.hp=p.ViewCurHP;v.maxHp=p.ViewMaxHP;v.mp=p.ViewCurMP;v.maxMp=p.ViewMaxMP;v.ag=p.ViewCurBP;v.maxAg=p.ViewMaxBP;v.sd=p.ViewCurSD;v.maxSd=p.ViewMaxSD;v.ready=true;}}
  if(b[off]==0xF3&&b.size()>off+1&&b[off+1]==5){require(b.size()==sizeof(PMSG_LEVEL_UP_SEND),"Invalid level-up event");PMSG_LEVEL_UP_SEND p{};std::memcpy(&p,b.data(),sizeof(p));auto& v=world.vitals;v.level=p.Level;v.hp=v.maxHp=p.ViewMaxHP;v.mp=v.maxMp=p.ViewMaxMP;v.ag=v.maxAg=p.ViewMaxBP;v.sd=v.maxSd=p.ViewMaxSD;v.experience=p.ViewExperience;v.nextExperience=p.ViewNextExperience;world.combat.levelRevision++;}
  if(b[off]==0x26){require(b.size()==sizeof(PMSG_LIFE_SEND),"Invalid HUD life update");PMSG_LIFE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.type==0xFE){world.vitals.maxHp=p.ViewHP;world.vitals.maxSd=p.ViewSD;}else if(p.type==0xFF){world.vitals.hp=p.ViewHP;world.vitals.sd=p.ViewSD;}}
  if(b[off]==0x27){require(b.size()==sizeof(PMSG_MANA_SEND),"Invalid HUD mana update");PMSG_MANA_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.type==0xFE){world.vitals.maxMp=p.ViewMP;world.vitals.maxAg=p.ViewBP;}else if(p.type==0xFF){world.vitals.mp=p.ViewMP;world.vitals.ag=p.ViewBP;}}
  if(b[off]==PROTOCOL_CODE2){require(b.size()==sizeof(PMSG_DAMAGE_SEND),"Invalid HUD damage update");PMSG_DAMAGE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if((((unsigned(p.index[0])&0x7F)<<8)|p.index[1])==index){world.vitals.hp=p.ViewCurHP;world.vitals.sd=p.ViewCurSD;}}
  if(b[off]==0x9C){require(b.size()==sizeof(PMSG_REWARD_EXPERIENCE_SEND),"Invalid HUD experience update");PMSG_REWARD_EXPERIENCE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));world.vitals.experience=p.ViewExperience;world.vitals.nextExperience=p.ViewNextExperience;}
  auto receipt=[&](unsigned kind,unsigned amount,const Item& item=Item{}){world.receipts.push_back({++world.receiptRevision,kind,amount,item});if(world.receipts.size()>64)world.receipts.erase(world.receipts.begin());};
  if(b[off]==0x16){PMSG_MONSTER_DIE_SEND p{};require(b.size()==sizeof(p),"Invalid experience receipt");std::memcpy(&p,b.data(),sizeof(p));unsigned exp=(unsigned(p.experience[0])<<8)|p.experience[1];if(exp)receipt(0,exp);}
  if(b[off]==0x9C){PMSG_REWARD_EXPERIENCE_SEND p{};require(b.size()==sizeof(p),"Invalid experience receipt");std::memcpy(&p,b.data(),sizeof(p));unsigned exp=(unsigned(p.experience[0])<<16)|p.experience[1];if(exp)receipt(0,exp);}
  if(b[off]==0x22){require(b.size()==sizeof(PMSG_ITEM_GET_SEND),"Invalid item/money reply");if(b[off+1]==0xFE){auto previous=world.money;world.money=(std::uint32_t(b[off+2])<<24)|(std::uint32_t(b[off+3])<<16)|(std::uint32_t(b[off+4])<<8)|b[off+5];if(world.moneyReady&&world.money>previous)receipt(2,world.money-previous);world.moneyReady=true;world.npc.inventoryMoney=world.money;world.npc.moneyReady=true;}else if(b[off+1]<0xFD){std::array<std::uint8_t,12> raw;std::copy_n(b.begin()+off+2,12,raw.begin());receipt(1,1,decode_item(raw));}else if(b[off+1]==0xFD&&pendingPickup)receipt(1,1,*pendingPickup);pendingPickup.reset();}

  if(b[off]==0x33){require(b.size()==sizeof(PMSG_ITEM_SELL_SEND),"Invalid sale reply");PMSG_ITEM_SELL_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.result==1){world.money=p.money;world.moneyReady=true;world.npc.inventoryMoney=p.money;world.npc.moneyReady=true;}}
  if(b[off]==0x34){require(b.size()==sizeof(PMSG_ITEM_REPAIR_SEND),"Invalid repair reply");PMSG_ITEM_REPAIR_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.money){world.money=p.money;world.moneyReady=true;world.npc.inventoryMoney=p.money;world.npc.moneyReady=true;}}
  if(b[off]==0xF3&&b.size()>off+1&&b[off+1]==0x11){require(b.size()>=6,"Invalid skill list");unsigned count=b[4],entries=count>=0xFE?1:count;require(b.size()==6+entries*4,"Invalid skill list count");if(b[5]==0){auto next=world.skills;if(count<0xFE)next.clear();for(unsigned i=0;i<entries;i++){auto n=6+i*4;unsigned slot=b[n],skill=unsigned(b[n+1])|(unsigned(b[n+2])<<8);if(count==0xFF)next.erase(slot);else next[slot]=skill;}world.skills=std::move(next);world.skillsReady=true;}}
  auto previousCast=world.combat.castRevision;apply_combat_packet(world.combat,b,index);if(world.combat.castRevision>previousCast&&world.combat.skill==9){spirits=std::make_unique<EvilSpirits>(world.x+.5,world.y+.5,unsigned(world.combat.castRevision));spiritTime=std::chrono::steady_clock::now();spiritHitFrame=0;}

  if(b[off]==0x81){require(b.size()==sizeof(PMSG_WAREHOUSE_MONEY_SEND),"Invalid warehouse balance");PMSG_WAREHOUSE_MONEY_SEND info{};std::memcpy(&info,b.data(),sizeof(info));world.npc.moneyReady=true;world.npc.warehouseMoney=info.WarehouseMoney;world.npc.inventoryMoney=info.InventoryMoney;world.money=info.InventoryMoney;world.moneyReady=true;}
  if(b[off]==0x83){require(b.size()==sizeof(PMSG_WAREHOUSE_STATE_SEND),"Invalid warehouse lock state");world.npc.lockState=b[off+1];}
  if(b[off]==0x30){require(b.size()==sizeof(PMSG_NPC_TALK_SEND),"Invalid NPC reply");world.npc.replied=true;world.npc.result=b[off+1];}
  if(b[off]==0xF9&&b.size()>off+1&&b[off+1]==1){require(b.size()==sizeof(PMSG_QUEST_WORLD_NPC_TALK_SEND),"Invalid quest NPC reply");PMSG_QUEST_WORLD_NPC_TALK_SEND info{};std::memcpy(&info,b.data(),sizeof(info));world.npc.replied=true;world.npc.result=0xF9;world.npc.npcClass=info.MonsterClass;world.npc.contribution=info.contribution;}
  if(b[off]==0x31&&b[0]==0xC2){require(b.size()>=sizeof(PMSG_SHOP_ITEM_LIST_SEND),"Truncated NPC item list");require(b.size()==sizeof(PMSG_SHOP_ITEM_LIST_SEND)+b[5]*sizeof(PMSG_SHOP_ITEM_LIST),"Invalid NPC item list");world.npc.items=std::make_shared<const ItemSlots>(decode_item_list(b,sizeof(PMSG_SHOP_ITEM_LIST_SEND),world.npc.result==2?WAREHOUSE_SIZE:120));world.npc.itemCount=world.npc.items->size();world.npc.itemsReady=true;}
  if(!leaving&&b[off]==0xF1&&b.size()>off+1&&b[off+1]==2)throw std::runtime_error("GameServer closed the session");

  if(b[off]==0xF3&&b.size()>off+1&&b[off+1]==0xE1){require(b.size()==sizeof(PMSG_NEW_CHARACTER_CALC_SEND),"Invalid character stats packet");PMSG_NEW_CHARACTER_CALC_SEND info{};std::memcpy(&info,b.data(),sizeof(info));require(info.ViewPhysiSpeed<=65535&&info.ViewMagicSpeed<=65535,"Attack speed outside heartbeat bounds");world.physicalSpeed=info.ViewPhysiSpeed;world.magicSpeed=info.ViewMagicSpeed;world.statsReady=true;}
  if(b[off]==PROTOCOL_CODE3){require(b.size()==sizeof(PMSG_POSITION_SEND),"Invalid position correction");PMSG_POSITION_SEND info{};std::memcpy(&info,b.data(),sizeof(info));if(((unsigned(info.index[0])<<8)|info.index[1])==index){world.x=info.x;world.y=info.y;}}
  if(b[off]==PROTOCOL_CODE1){require(b.size()==sizeof(PMSG_MOVE_SEND),"Invalid move acknowledgement");PMSG_MOVE_SEND info{};std::memcpy(&info,b.data(),sizeof(info));if(((unsigned(info.index[0])<<8)|info.index[1])==index){world.x=info.x;world.y=info.y;world.direction=info.dir>>4;}}
 }
 Bytes receive(){require(socket!=nullptr,"Session is disconnected");auto b=normalize(socket->frame(&profile));observe(b);return b;}
 Bytes await(unsigned head,unsigned sub){
  auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(12);
  for(unsigned i=0;i<256&&std::chrono::steady_clock::now()<deadline;i++){
   auto b=receive();auto off=b[0]==0xC2?3u:2u;
   if(b.size()>off+1&&b[off]==head&&b[off+1]==sub)return b;
   require(!(b.size()>3&&b[2]==0xF1&&b[3]==2),"GameServer closed the session");
  }
  throw std::runtime_error("Expected GameServer response not received");
 }
};
Session::Session(const Profile& p,unsigned code):impl(std::make_unique<Impl>(p,code)){}
Session::~Session()=default;
unsigned Session::login(const std::string& account,const std::string& password){
 require(!impl->logged,"Session already logged in");impl->send(login_packet(impl->profile,account,password,impl->tick()));auto packet=impl->await(0xF1,1);
 require(packet.size()==5,"Invalid login result");impl->logged=packet[4]==1;return packet[4];
}
CharacterList Session::characters(){require(impl->logged&&!impl->entered,"Character list requires a logged-in lobby session");impl->send({0xC1,4,0xF3,0});auto list=parse_characters(impl->await(0xF3,0));auto policy=impl->await(0xDE,0);require(policy.size()==sizeof(PMSG_CHARACTER_CREATION_ENABLE_SEND),"Invalid EX603 character creation policy");PMSG_CHARACTER_CREATION_ENABLE_SEND info{};std::memcpy(&info,policy.data(),sizeof(info));list.classCode=info.result;list.deleteMaxLevel=info.CharacterDeleteMaxLevel;list.deletePolicyReady=true;return list;}
unsigned Session::create(const std::string& name,unsigned cls){
 require(impl->logged&&!impl->entered,"Character creation requires a lobby session");name_check(name);require(name.size()>=4,"Character name must contain at least 4 characters");require(cls<=96&&cls%16==0,"Invalid EX603 class code");
 PMSG_CHARACTER_CREATE_RECV packet{};packet.header.set(0xF3,1,sizeof(packet));std::memcpy(packet.name,name.data(),name.size());packet.Class=cls;impl->send(bytes(packet));
 auto response=impl->await(0xF3,1);require(response.size()==sizeof(PMSG_CHARACTER_CREATE_SEND),"Invalid create-character result");return response[4];
}
Bytes delete_character_packet(const std::string& name,const std::string& personalCode){name_check(name);require(personalCode.size()<=10,"Personal code exceeds wire limit");for(unsigned char c:personalCode)require(c>='0'&&c<='9',"Personal code must contain digits");PMSG_CHARACTER_DELETE_RECV packet{};packet.header.set(0xF3,2,sizeof(packet));std::memcpy(packet.name,name.data(),name.size());std::memcpy(packet.PersonalCode,personalCode.data(),personalCode.size());return bytes(packet);}
unsigned Session::erase(const std::string& name,const std::string& personalCode){require(impl->logged&&!impl->entered,"Deletion requires a logged-in lobby session");auto list=characters();auto found=std::find_if(list.characters.begin(),list.characters.end(),[&](const auto& c){return c.name==name;});require(found!=list.characters.end(),"Character is not in this account");require(found->guildStatus==255,"Character belongs to a guild");require(!(found->control&18),"Character deletion is blocked");require(list.deletePolicyReady&&found->level<list.deleteMaxLevel,"Character exceeds server deletion level limit");impl->send(delete_character_packet(name,personalCode));auto response=impl->await(0xF3,2);require(response.size()==sizeof(PMSG_CHARACTER_DELETE_SEND),"Invalid character deletion result");return response[4];}
WorldEntry Session::select(const std::string& name){
 require(impl->logged&&!impl->entered,"Character selection requires a lobby session");name_check(name);
 auto list=characters();auto selected=std::find_if(list.characters.begin(),list.characters.end(),[&](auto& c){return c.name==name;});require(selected!=list.characters.end(),"Character is not in this account's list");
 PMSG_CHARACTER_INFO_RECV packet{};packet.header.set(0xF3,3,sizeof(packet));std::memcpy(packet.name,name.data(),name.size());impl->send(bytes(packet));
 auto response=impl->await(0xF3,3);require(response.size()==sizeof(PMSG_CHARACTER_INFO_SEND),"Invalid character world-entry response");
 PMSG_CHARACTER_INFO_SEND info{};std::memcpy(&info,response.data(),sizeof(info));impl->entered=true;impl->world.map=info.Map;impl->world.x=info.X;impl->world.y=info.Y;impl->world.direction=info.Dir;impl->world.money=info.Money;impl->world.moneyReady=true;
 impl->world.attributes.level=selected->level;auto appearance=selected->appearance.at(0);impl->world.attributes.tier=(appearance&28)>=28?2:(appearance&16)?1:0;
 require((selected->appearance.at(0)>>5)==info.Class,"Selected character class differs from world-entry response");
 return {info.Map,info.X,info.Y,info.Dir,std::move(response),info.Class,*selected};
}
bool apply_character_attributes(EquipmentAttributes& a,const Bytes& b){if(b.size()<4||(b[0]!=0xC1&&b[0]!=0xC3)||b[2]!=0xF3)return false;auto base=[&](auto& p){a.base={p.ViewStrength,p.ViewDexterity,p.ViewEnergy,p.ViewVitality,p.ViewLeadership};a.points=p.ViewPoint;a.ready=true;};switch(b[3]){case 3:{require(b.size()==sizeof(PMSG_CHARACTER_INFO_SEND),"Invalid character attributes");PMSG_CHARACTER_INFO_SEND p{};std::memcpy(&p,b.data(),sizeof(p));base(p);a.baseClass=p.Class;return true;}case 0xE0:{require(b.size()==sizeof(PMSG_NEW_CHARACTER_INFO_SEND),"Invalid character attribute refresh");PMSG_NEW_CHARACTER_INFO_SEND p{};std::memcpy(&p,b.data(),sizeof(p));base(p);a.level=p.Level;return true;}case 0xE1:{require(b.size()==sizeof(PMSG_NEW_CHARACTER_CALC_SEND),"Invalid bonus attributes");PMSG_NEW_CHARACTER_CALC_SEND p{};std::memcpy(&p,b.data(),sizeof(p));a.bonus={p.ViewAddStrength,p.ViewAddDexterity,p.ViewAddEnergy,p.ViewAddVitality,p.ViewAddLeadership};a.combat={p.ViewPhysiDamageMin,p.ViewPhysiDamageMax,p.ViewMagicDamageMin,p.ViewMagicDamageMax,p.ViewDefense,p.ViewDefenseSuccessRate,p.ViewPhysiSpeed,p.ViewAttackSuccessRate};auto damage=[](std::uint32_t value,std::uint32_t mul,std::uint32_t div){std::uint64_t v=value;v+=v*mul/100;v-=std::min(v,v*div/100);return std::uint32_t(std::min(v,std::uint64_t(UINT32_MAX)));};a.combat[0]=damage(p.ViewPhysiDamageMin,p.ViewMulPhysiDamage,p.ViewDivPhysiDamage);a.combat[1]=damage(p.ViewPhysiDamageMax,p.ViewMulPhysiDamage,p.ViewDivPhysiDamage);a.combat[2]=damage(p.ViewMagicDamageMin,p.ViewMulMagicDamage,p.ViewDivMagicDamage);a.combat[3]=damage(p.ViewMagicDamageMax,p.ViewMulMagicDamage,p.ViewDivMagicDamage);a.combatReady=true;return true;}case 6:{require(b.size()==sizeof(PMSG_LEVEL_UP_POINT_SEND),"Invalid point attributes");PMSG_LEVEL_UP_POINT_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.result>=16&&p.result<=20)base(p);return true;}case 5:{require(b.size()==sizeof(PMSG_LEVEL_UP_SEND),"Invalid level attributes");PMSG_LEVEL_UP_SEND p{};std::memcpy(&p,b.data(),sizeof(p));a.level=p.Level;a.points=p.ViewPoint;return true;}default:return false;}}
Bytes path_packet(unsigned x,unsigned y,const std::vector<unsigned>& directions){
 require(x<256&&y<256&&!directions.empty()&&directions.size()<=14,"Path outside EX603 bounds");
 PMSG_MOVE_RECV packet{};packet.header.set(PROTOCOL_CODE1,sizeof(packet));packet.x=x;packet.y=y;packet.path[0]=(directions.front()<<4)|directions.size();
 constexpr int dx[8]={-1,0,1,1,1,0,-1,-1},dy[8]={-1,-1,-1,0,1,1,1,0};int tx=x,ty=y;
 for(unsigned i=0;i<directions.size();i++){auto d=directions[i];require(d<8,"Invalid path direction");tx+=dx[d];ty+=dy[d];require(tx>=0&&ty>=0&&tx<256&&ty<256,"Path leaves map");packet.path[1+i/2]|=d<<(i%2?0:4);}return bytes(packet);
}
Bytes move_packet(unsigned x,unsigned y,unsigned direction){return path_packet(x,y,{direction});}
WorldState Session::pump(){
 require(impl->socket!=nullptr,"Session disconnected");
 for(unsigned i=0;i<64;i++){auto packet=impl->socket->try_frame(&impl->profile);if(!packet)break;auto decoded=impl->normalize(std::move(*packet));impl->observe(decoded);}
 if(impl->spirits){auto now=std::chrono::steady_clock::now();double dt=std::chrono::duration<double>(now-impl->spiritTime).count();impl->spiritTime=now;impl->spirits->accumulator+=std::min(dt,.25);while(impl->spirits->accumulator>=.04&&impl->spirits->frame<50){impl->spirits->accumulator-=.04;impl->spirits->step(impl->world.x+.5,impl->world.y+.5);if(impl->spirits->hitTick()){PMSG_MULTI_SKILL_ATTACK_RECV p{};p.header.setE(PROTOCOL_CODE4,sizeof(p));p.skill[1]=9;p.x=impl->world.x;p.y=impl->world.y;p.serial=impl->skillSerial;Bytes packet(sizeof(p));for(const auto& pair:impl->viewport.entities){const auto& e=pair.second;if(e.dead||e.kind!=EntityKind::Monster)continue;if(std::hypot(double(e.tx)-impl->world.x,double(e.ty)-impl->world.y)>6)continue;packet.push_back(e.index>>8);packet.push_back(e.index);packet.push_back(0);if(++p.count==5)break;}if(std::getenv("MU_NATIVE_CLIENT_COMBAT_TRACE"))std::fprintf(stderr,"[Evil hit wave] frame=%u self=%u,%u candidates=%u head0=%.2f,%.2f\n",impl->spirits->frame,impl->world.x,impl->world.y,p.count,impl->spirits->heads[0].p.x,impl->spirits->heads[0].p.y);if(p.count){p.header.size=packet.size();std::memcpy(packet.data(),&p,sizeof(p));impl->send(packet);}}}if(impl->spirits->frame>=50)impl->spirits.reset();}
 if(std::chrono::steady_clock::now()-impl->heartbeat>=std::chrono::seconds(5))keepalive();impl->publish();return impl->world;
}
Bytes npc_talk_packet(unsigned index){require(index<0x4000,"Invalid NPC index");return {0xC3,5,0x30,std::uint8_t(index>>8),std::uint8_t(index)};}
bool Session::addStat(unsigned type){require(impl->logged&&impl->entered&&type<5,"Invalid stat allocation");PMSG_LEVEL_UP_POINT_RECV p{};p.header.set(0xF3,6,sizeof(p));p.type=type;impl->send(bytes(p));auto reply=impl->await(0xF3,6);require(reply.size()==sizeof(PMSG_LEVEL_UP_POINT_SEND),"Invalid stat allocation response");return reply[4]==16+type;}
unsigned Session::buyItem(unsigned slot){require(impl->logged&&impl->entered&&impl->world.npc.replied&&impl->world.npc.result==0&&impl->world.npc.itemsReady,"Shop is not open");require(slot<120&&impl->world.npc.items&&impl->world.npc.items->count(slot),"Shop slot is empty");impl->send({0xC3,4,0x32,std::uint8_t(slot)});auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(4);while(std::chrono::steady_clock::now()<deadline){auto packet=impl->receive();if(packet.size()>2&&packet[2]==0x32){impl->publish();return packet[3];}}disconnect();throw std::runtime_error("Purchase confirmation timeout; reconnect to reload inventory");}
bool Session::sellItem(unsigned slot){require(impl->logged&&impl->entered&&impl->world.npc.replied&&impl->world.npc.result==0,"Shop is not open");pump();require(slot<76&&impl->inventory.items.count(slot),"Sale source is empty");auto original=impl->inventory.items.at(slot);impl->send({0xC3,4,0x33,std::uint8_t(slot)});auto end=std::chrono::steady_clock::now()+std::chrono::seconds(4);while(std::chrono::steady_clock::now()<end){auto b=impl->receive();if(b.size()>2&&b[2]==0x33){bool accepted=b[3]==1;if(accepted){auto found=impl->inventory.items.find(slot);require(found==impl->inventory.items.end()||found->second.wire==original.wire,"Sale source changed before ACK");if(found!=impl->inventory.items.end()){impl->inventory.items.erase(found);impl->inventory.revision++;}}impl->publish();return accepted;}}disconnect();throw std::runtime_error("Sale confirmation timeout; reconnect to reload inventory");}
bool Session::repairItem(unsigned slot){require(impl->logged&&impl->entered&&impl->world.npc.replied&&impl->world.npc.result==0,"Shop is not open");pump();require(slot==255||(slot<76&&impl->inventory.items.count(slot)),"Repair source is empty");auto before=impl->inventory;auto money=impl->world.money;impl->send({0xC3,5,0x34,std::uint8_t(slot),0});auto until=std::chrono::steady_clock::now()+std::chrono::seconds(1);while(std::chrono::steady_clock::now()<until){pump();std::this_thread::sleep_for(std::chrono::milliseconds(20));}impl->publish();return impl->inventory.revision!=before.revision||impl->world.money!=money;}
bool Session::useBook(unsigned slot){require(impl->logged&&impl->entered&&slot>=12&&slot<76,"Book use requires inventory slot");pump();auto found=impl->inventory.items.find(slot);require(found!=impl->inventory.items.end(),"Book source is empty");unsigned group=found->second.index/512,type=found->second.index%512;require(group==15||(group==12&&((type>=7&&type<=24)||type==35||(type>=44&&type<=48))),"Selected item is not a skill book");impl->send({0xC3,6,0x26,std::uint8_t(slot),0xFF,0});auto until=std::chrono::steady_clock::now()+std::chrono::seconds(1);while(std::chrono::steady_clock::now()<until){pump();std::this_thread::sleep_for(std::chrono::milliseconds(20));}impl->publish();return !impl->inventory.items.count(slot);}
bool apply_combat_packet(CombatState& combat,const Bytes& b,unsigned selfIndex){if(b.size()<3)return false;unsigned off=b[0]==0xC2?3:2;if(b.size()<=off)return false;
 auto record=[&](unsigned target,unsigned damage,unsigned type){combat.target=target;combat.damage=damage;combat.damageType=type;combat.damageRevision++;combat.hits.push_back({combat.damageRevision,target,damage,type});if(combat.hits.size()>64)combat.hits.erase(combat.hits.begin());};
  if(b[off]==0x18){require(b.size()==sizeof(PMSG_ACTION_SEND),"Invalid attack action reply");PMSG_ACTION_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(((unsigned(p.index[0])<<8)|p.index[1])==selfIndex&&p.action==120){combat.skill=0;combat.castTarget=((unsigned(p.target[0])&0x7F)<<8)|p.target[1];combat.castRevision++;}}
  if(b[off]==0x19){require(b.size()==sizeof(PMSG_SKILL_ATTACK_SEND),"Invalid skill cast reply");PMSG_SKILL_ATTACK_SEND p{};std::memcpy(&p,b.data(),sizeof(p));unsigned actor=(unsigned(p.index[0])<<8)|p.index[1];if((actor&0x7FFF)==selfIndex){combat.skill=(unsigned(p.skill[0])<<8)|p.skill[1];combat.castTarget=((unsigned(p.target[0])&0x7F)<<8)|p.target[1];combat.castRevision++;}}
  if(b[off]==0x1E){require(b.size()==sizeof(PMSG_DURATION_SKILL_ATTACK_SEND),"Invalid area cast reply");PMSG_DURATION_SKILL_ATTACK_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(((unsigned(p.index[0])<<8)|p.index[1])==selfIndex){combat.skill=(unsigned(p.skill[0])<<8)|p.skill[1];combat.castTarget=0;combat.castRevision++;}}
  if(b[off]==PROTOCOL_CODE2){require(b.size()==sizeof(PMSG_DAMAGE_SEND),"Invalid damage event");PMSG_DAMAGE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));record(((unsigned(p.index[0])&0x7F)<<8)|p.index[1],p.ViewDamageHP,p.type);}
  if(b[off]==0x16){require(b.size()==sizeof(PMSG_MONSTER_DIE_SEND),"Invalid monster death reply");PMSG_MONSTER_DIE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));combat.kills++;if(p.ViewDamageHP)record(((unsigned(p.index[0])&0x7F)<<8)|p.index[1],p.ViewDamageHP,0);}
if(b[off]==0x9C){require(b.size()==sizeof(PMSG_REWARD_EXPERIENCE_SEND),"Invalid killing damage event");PMSG_REWARD_EXPERIENCE_SEND p{};std::memcpy(&p,b.data(),sizeof(p));if(p.ViewDamageHP){combat.kills++;record(((unsigned(p.index[0])&0x7F)<<8)|p.index[1],p.ViewDamageHP,0);}}
return b[off]==0x1E||b[off]==0x18||b[off]==0x19||b[off]==PROTOCOL_CODE2||b[off]==0x16||b[off]==0x9C;}
Bytes normal_attack_packet(unsigned target,unsigned direction){require(target<0x4000&&direction<8,"Invalid attack target or direction");return {0xC1,7,PROTOCOL_CODE2,std::uint8_t(target>>8),std::uint8_t(target),120,std::uint8_t(direction)};}
void Session::attack(unsigned target,unsigned direction){require(impl->logged&&impl->entered,"Attack requires world entry");auto found=impl->viewport.entities.find(target);require(found!=impl->viewport.entities.end()&&found->second.kind==EntityKind::Monster&&!found->second.dead,"Monster target is unavailable");impl->send(normal_attack_packet(target,direction));}
Bytes skill_attack_packet(unsigned skill,unsigned target){require((skill==4||skill==17)&&target<0x4000,"Unsupported targeted skill or target");return {0xC3,8,0x19,std::uint8_t(skill>>8),std::uint8_t(skill),std::uint8_t(target>>8),std::uint8_t(target),0};}
void Session::castSkill(unsigned skill,unsigned target){require(impl->logged&&impl->entered,"Cast requires world entry");bool learned=false;for(auto& entry:impl->world.skills)if(entry.second==skill)learned=true;require(learned,"Skill has not been learned");auto found=impl->viewport.entities.find(target);require(found!=impl->viewport.entities.end()&&found->second.kind==EntityKind::Monster&&!found->second.dead,"Monster target is unavailable");impl->send(skill_attack_packet(skill,target));}
void Session::castAreaSkill(unsigned skill,unsigned direction){require(impl->logged&&impl->entered&&skill==9&&direction<8,"Invalid area skill");bool learned=false;for(const auto& e:impl->world.skills)learned|=e.second==skill;require(learned,"Skill has not been learned");PMSG_DURATION_SKILL_ATTACK_RECV p{};p.header.setE(0x1E,sizeof(p));p.skill[1]=skill;p.x=impl->world.x;p.y=impl->world.y;p.dir=direction*32;p.index[0]=p.index[1]=0xFF;p.MagicKey=++impl->skillSerial;auto begin=reinterpret_cast<const std::uint8_t*>(&p);impl->send(Bytes(begin,begin+sizeof(p)));}
bool Session::dropItem(unsigned slot){require(impl->logged&&impl->entered&&slot>=12&&slot<76&&impl->inventory.items.count(slot),"Invalid bag item to drop");impl->send({0xC3,6,0x23,std::uint8_t(impl->world.x),std::uint8_t(impl->world.y),std::uint8_t(slot)});Bytes reply;auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(4);do{require(std::chrono::steady_clock::now()<deadline,"Drop confirmation timeout");reply=impl->receive();}while(reply.size()<3||reply[2]!=0x23);impl->publish();require(reply.size()==sizeof(PMSG_ITEM_DROP_SEND),"Invalid drop acknowledgement");return reply[3]==1;}
bool Session::pickItem(unsigned index){require(impl->logged&&impl->entered&&impl->viewport.items.count(index),"Ground item unavailable");impl->pendingPickup=impl->viewport.items.at(index).item;impl->send({0xC3,5,0x22,std::uint8_t(index>>8),std::uint8_t(index)});Bytes reply;auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(4);do{require(std::chrono::steady_clock::now()<deadline,"Pickup confirmation timeout");reply=impl->receive();}while(reply.size()<3||reply[2]!=0x22);impl->publish();require(reply.size()==sizeof(PMSG_ITEM_GET_SEND),"Invalid pickup acknowledgement");return reply[3]!=0xFF;}
void Session::talk(unsigned index){require(impl->logged&&impl->entered&&index<0x4000,"Invalid NPC interaction");impl->world.npc={};auto npc=impl->viewport.entities.find(index);if(npc!=impl->viewport.entities.end())impl->world.npc.npcClass=npc->second.type;impl->send(npc_talk_packet(index));}
void Session::closeTalk(){if(impl->socket){impl->send({0xC1,3,0x31});impl->world.npc={};}}
bool Session::moveItem(const ItemMove& move){require(impl->logged&&impl->entered,"Item transfer requires world entry");pump();bool warehouse=move.sourceContainer==2||move.targetContainer==2;if(warehouse)require(impl->world.npc.replied&&impl->world.npc.result==2&&impl->world.npc.itemsReady&&impl->world.npc.lockState==0,"Warehouse is not open or unlocked");ItemSlots vault=impl->world.npc.items?*impl->world.npc.items:ItemSlots{};const auto& source=move.sourceContainer==0?impl->inventory.items:vault;auto found=source.find(move.sourceSlot);require(found!=source.end(),"Transfer source is empty");auto original=found->second;impl->send(item_move_packet(move,original));auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(4);while(std::chrono::steady_clock::now()<deadline){auto packet=impl->receive();if(packet.size()>2&&packet[2]==0x24){bool accepted=commit_item_move(impl->inventory,vault,move,original,packet);if(accepted&&warehouse){impl->world.npc.items=std::make_shared<const ItemSlots>(std::move(vault));impl->world.npc.itemCount=impl->world.npc.items->size();}impl->publish();return accepted;}}disconnect();throw std::runtime_error("Item transfer confirmation timeout; reconnect to reload authoritative inventory");}
WorldState Session::state()const{impl->publish();return impl->world;}
WorldState Session::move(unsigned direction){pump();return movePath(impl->world.x,impl->world.y,{direction});}
WorldState Session::movePath(unsigned startX,unsigned startY,const std::vector<unsigned>& directions){
 require(impl->logged&&impl->entered,"Movement requires world entry");auto packet=path_packet(startX,startY,directions);pump();
 auto now=std::chrono::steady_clock::now();require(impl->lastMove==std::chrono::steady_clock::time_point::min()||now-impl->lastMove>=std::chrono::milliseconds(40),"Wait for the previous movement request");
 constexpr int dx[8]={-1,0,1,1,1,0,-1,-1},dy[8]={-1,-1,-1,0,1,1,1,0};int x=startX,y=startY;for(auto d:directions){x+=dx[d];y+=dy[d];}
 impl->send(packet);impl->lastMove=now;
 auto deadline=now+std::chrono::seconds(3);
 while(std::chrono::steady_clock::now()<deadline){auto b=impl->receive();if(b[0]==0xC1&&b[2]==PROTOCOL_CODE3&&((unsigned(b[3])<<8)|b[4])==impl->index)throw std::runtime_error("Server corrected the movement position");if(b[0]==0xC1&&b[2]==PROTOCOL_CODE1&&((unsigned(b[3])<<8)|b[4])==impl->index){require(impl->world.x==unsigned(x)&&impl->world.y==unsigned(y),"Server rejected or corrected the movement");std::ofstream metrics("/tmp/mu-client-network-performance.log",std::ios::app);impl->publish();metrics<<"steps="<<directions.size()<<" ack_ms="<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-now).count()<<"\n";return impl->world;}}
 throw std::runtime_error("Movement confirmation timeout");
}
void Session::keepalive(){
 require(impl->logged,"Keepalive requires login");
 if(impl->entered&&!impl->world.statsReady)return;
 PMSG_LIVE_CLIENT_RECV packet{};packet.header.setE(0x0E,sizeof(packet));packet.TickCount=impl->tick();packet.PhysiSpeed=impl->world.physicalSpeed;packet.MagicSpeed=impl->world.magicSpeed;impl->send(bytes(packet));impl->heartbeat=std::chrono::steady_clock::now();
}
Bytes logout_packet(unsigned type){require(type<=2,"Invalid logout target");PMSG_CLOSE_CLIENT_RECV packet{};packet.header.setE(0xF1,2,sizeof(packet));packet.type=type;return bytes(packet);}
void Session::setNoticeHandler(std::function<void(const ServerNotice&)> handler){impl->noticeHandler=std::move(handler);}
void Session::requestLogout(unsigned type){
 require(impl->logged&&type<=2,"Invalid logout request");
 impl->leaving=true;
 try{impl->send(logout_packet(type));auto reply=impl->await(0xF1,2);
  require(reply.size()==5&&reply[4]==type,"Unexpected logout response");
  if(type==1){impl->entered=false;impl->world={};impl->viewport={};impl->inventory={};impl->lastMove=std::chrono::steady_clock::time_point::min();}else disconnect();
  impl->leaving=false;
 }catch(...){impl->leaving=false;throw;}
}
void Session::disconnect(){impl->socket.reset();impl->logged=false;impl->entered=false;}

}
