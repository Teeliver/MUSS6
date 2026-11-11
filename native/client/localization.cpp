#include "localization.hpp"
#include <stdexcept>
#include <cctype>
namespace mu::ui {
Texts decode_texts(const mu::assets::Bytes& bytes){
 std::size_t position=0;auto u32=[&](){if(position+4>bytes.size())throw std::runtime_error("Truncated text table");std::uint32_t value=0;for(unsigned i=0;i<4;i++)value|=std::uint32_t(bytes[position++])<<(i*8);return value;};
 if(bytes.size()<6||bytes[0]!=0x47||bytes[1]!=0x54)throw std::runtime_error("Invalid GlobalText signature");position=2;auto count=u32();if(count>100000)throw std::runtime_error("Text table count exceeds bounds");Texts result;constexpr unsigned char key[]={0xFC,0xCF,0xAB};
 for(unsigned i=0;i<count;i++){auto index=u32(),length=u32();if(length>65536||length>bytes.size()-position)throw std::runtime_error("Invalid text record size");std::string text(length,0);for(unsigned j=0;j<length;j++)text[j]=bytes[position+j]^key[j%3];position+=length;if(!text.empty()&&text.back()==0)text.pop_back();if(text.find(char(0))!=std::string::npos||!result.emplace(index,std::move(text)).second)throw std::runtime_error("Invalid or duplicate text record");}
 if(position!=bytes.size())throw std::runtime_error("Trailing bytes in text table");return result;
}
std::filesystem::path local_data_file(const std::filesystem::path& data,const std::string& language,const std::string& stem){if(language.size()!=3)throw std::runtime_error("Language must be a three-letter code");std::string code;for(unsigned char c:language){if(!std::isalpha(c)||c>127)throw std::runtime_error("Invalid language code");code+=char(std::tolower(c));}auto directory=code;directory[0]=char(std::toupper(directory[0]));return data/"Local"/directory/(stem+"_"+code+".bmd");}
Texts load_texts(const std::filesystem::path& data,const std::string& language){return decode_texts(mu::assets::read_file(local_data_file(data,language,"Text")));}
std::string format_gold(std::uint32_t amount){auto text=std::to_string(amount);for(int i=int(text.size())-3;i>0;i-=3)text.insert(i,",");return text;}
std::array<unsigned,3> gold_color(std::uint32_t amount){if(amount>=10000000)return {255,0,0};if(amount>=1000000)return {255,150,0};if(amount>=100000)return {0,201,24};return {255,220,150};}
}
