#pragma once
#include "assets.hpp"
#include "character_scene.hpp"
#include <memory>
#include <functional>
namespace mu::lobby {
struct Part {mu::assets::Mesh mesh;std::vector<mu::assets::Vec3> posed;std::shared_ptr<const mu::assets::Image> image;};
struct Model {std::vector<Part> parts;};
struct Ground {std::vector<mu::assets::Vec3> positions;std::vector<std::array<float,2>> uv;std::vector<std::array<float,4>> colors;std::shared_ptr<const mu::assets::Image> image;bool overlay=false;};
struct Waypoint {float x,y,z;unsigned delay;float speed,distance;};
struct Scene {std::shared_ptr<const mu::assets::Image> selectionImage;unsigned world=74;mu::assets::Terrain terrain;mu::assets::ObjectFile placements;std::map<unsigned,Model> models;std::vector<Waypoint> camera;std::vector<Ground> ground;std::size_t triangles=0;std::vector<unsigned> missingModelTypes;};
std::shared_ptr<const Scene> load(const std::string& root,const std::function<void(unsigned,unsigned)>& progress={},unsigned world=74);
}
#ifdef __OBJC__
#import <SceneKit/SceneKit.h>
@interface MuLobbyScene:SCNView<SCNSceneRendererDelegate>
- (instancetype)initWithData:(std::shared_ptr<const mu::lobby::Scene>)data;
- (void)setActive:(BOOL)active;
- (void)setCharacters:(const std::vector<mu::lobby::CharacterVisual>&)characters;
- (NSInteger)characterRowAtPoint:(NSPoint)point;
- (NSPoint)characterAnchor:(unsigned)row;
- (void)selectCharacter:(NSInteger)row;
@property(nonatomic,copy) void (^onCharacter)(unsigned row,BOOL enter);
@end
#endif
