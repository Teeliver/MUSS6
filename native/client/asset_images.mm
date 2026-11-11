#include "asset_images.hpp"
#import <Cocoa/Cocoa.h>
#import <ImageIO/ImageIO.h>
#include <algorithm>
#include <stdexcept>
#include <mutex>
namespace mu::viewer {namespace {void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}}
mu::assets::Image decode_texture(const std::filesystem::path& path){
 auto b=mu::assets::read_file(path);auto ext=path.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
 if(ext==".ozt")return mu::assets::decode_tga(b);
 need(ext==".ozj","Unsupported texture extension");auto jpg=mu::assets::jpeg_payload(b);
 NSData* data=[NSData dataWithBytes:jpg.data() length:jpg.size()];CGImageSourceRef src=CGImageSourceCreateWithData((__bridge CFDataRef)data,nullptr);need(src!=nullptr,"Could not decode OZJ image");
 CGImageRef cg=CGImageSourceCreateImageAtIndex(src,0,nullptr);CFRelease(src);need(cg!=nullptr,"Invalid JPEG payload");
 auto width=CGImageGetWidth(cg),height=CGImageGetHeight(cg);
 if(width==0||height==0||width>4096||height>4096){CGImageRelease(cg);throw std::runtime_error("JPEG dimensions out of bounds");}
 mu::assets::Image out{unsigned(width),unsigned(height),mu::assets::Bytes(width*height*4,0)};
 auto color=CGColorSpaceCreateDeviceRGB();auto ctx=CGBitmapContextCreate(out.rgba.data(),width,height,8,width*4,color,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);CGColorSpaceRelease(color);
 if(!ctx){CGImageRelease(cg);throw std::runtime_error("Could not decode texture pixels");}
 CGContextTranslateCTM(ctx,0,height);CGContextScaleCTM(ctx,1,-1);CGContextDrawImage(ctx,CGRectMake(0,0,width,height),cg);CGContextRelease(ctx);CGImageRelease(cg);return out;
}
std::filesystem::path resolve_model_texture(const std::filesystem::path& directory,const std::string& name){
 auto p=std::filesystem::path(name);auto ext=p.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
 need(ext==".jpg"||ext==".tga","Unsupported BMD texture reference");p.replace_extension(ext==".jpg"?".OZJ":".OZT");
 auto target=p.filename().string();std::transform(target.begin(),target.end(),target.begin(),::tolower);
 for(auto& entry:std::filesystem::directory_iterator(directory)){auto candidate=entry.path().filename().string();std::transform(candidate.begin(),candidate.end(),candidate.begin(),::tolower);if(candidate==target)return entry.path();}
 throw std::runtime_error("Required original model texture was not found: "+name);
}
NSImage* load_interface_image(const std::filesystem::path& directory,const char* file){static std::mutex cacheMutex;static NSMutableDictionary<NSString*,NSImage*>* cache=[NSMutableDictionary dictionary];std::lock_guard<std::mutex> guard(cacheMutex);auto path=resolve_model_texture(directory,file);NSString* key=[NSString stringWithUTF8String:path.string().c_str()];if(NSImage* cached=cache[key])return cached;auto image=decode_texture(path);NSData* bytes=[NSData dataWithBytes:image.rgba.data() length:image.rgba.size()];auto provider=CGDataProviderCreateWithCFData((__bridge CFDataRef)bytes);auto color=CGColorSpaceCreateDeviceRGB();auto cg=CGImageCreate(image.width,image.height,8,32,image.width*4,color,kCGImageAlphaLast|kCGBitmapByteOrder32Big,provider,nullptr,YES,kCGRenderingIntentDefault);if(!cg){CGColorSpaceRelease(color);CGDataProviderRelease(provider);throw std::runtime_error("Could not create interface image");}auto result=[[NSImage alloc] initWithCGImage:cg size:NSMakeSize(image.width,image.height)];CGImageRelease(cg);CGColorSpaceRelease(color);CGDataProviderRelease(provider);cache[key]=result;return result;}

}
