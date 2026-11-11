#pragma once
#include "entity_assets.hpp"
#include "viewport.hpp"
#ifdef __OBJC__
#import <SceneKit/SceneKit.h>
@interface MuGroundItemNode:SCNNode
@property(nonatomic) BOOL itemHovered;
@property(nonatomic) BOOL visualReady;
@property(nonatomic) std::uint64_t itemRevision;
@end
@interface MuCharacterRig:NSObject
@property(nonatomic,strong,readonly) SCNNode* node;
- (instancetype)initWithVisual:(std::shared_ptr<const mu::viewer::ActorVisual>)visual;
- (void)advance:(double)time;
@end
@interface MuViewportRenderer:NSObject
- (instancetype)initWithRoot:(SCNNode*)root terrain:(const mu::assets::Terrain&)terrain catalog:(const mu::viewer::ActorCatalog&)catalog;
- (NSArray*)preparationObjects;
- (void)releasePreparationObjects;
- (void)submit:(std::shared_ptr<const mu::client::ViewportSnapshot>)snapshot;
- (void)advance:(double)dt;
@end
#endif
