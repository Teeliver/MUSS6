#include <unistd.h>
#include <uuid/uuid.h>
#include <CommonCrypto/CommonDigest.h>
#include <cstdio>
#include <cstring>
int main(){uuid_t uuid{};timespec limit{1,0};if(gethostuuid(uuid,&limit)!=0)return 1;unsigned char digest[CC_SHA256_DIGEST_LENGTH]{};CC_SHA256(uuid,sizeof(uuid),digest);char identity[45]{};for(unsigned group=0;group<5;group++){if(group)identity[group*9-1]='-';for(unsigned byte=0;byte<4;byte++)std::snprintf(identity+group*9+byte*2,3,"%02X",digest[group*4+byte]);}std::puts(identity);std::memset(uuid,0,sizeof(uuid));std::memset(digest,0,sizeof(digest));std::memset(identity,0,sizeof(identity));}
