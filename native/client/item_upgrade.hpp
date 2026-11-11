#pragma once
#include "inventory.hpp"
#include <filesystem>
#import <SceneKit/SceneKit.h>
namespace mu::viewer {void apply_item_upgrade(SCNNode*,SCNGeometry*,SCNMaterial*,const std::filesystem::path&,const mu::client::Item&);}
