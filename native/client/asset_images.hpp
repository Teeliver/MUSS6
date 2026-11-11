#pragma once
#include "assets.hpp"
namespace mu::viewer {
mu::assets::Image decode_texture(const std::filesystem::path&);
std::filesystem::path resolve_model_texture(const std::filesystem::path& directory,const std::string& bmdTextureName);
}

#ifdef __OBJC__
@class NSImage;
namespace mu::viewer { NSImage* load_interface_image(const std::filesystem::path& directory,const char* filename); }
#endif
