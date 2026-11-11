#pragma once
#include "assets.hpp"
#include "character_resources.hpp"
#include "entity_assets.hpp"
#include "viewport.hpp"
#include <memory>
#include <functional>
#include "motion.hpp"
namespace mu::viewer {
struct Part { mu::assets::Mesh mesh;std::shared_ptr<const mu::assets::Image> image;bool groundContact=false;std::optional<mu::client::Item> equipment; };
struct EnvironmentBatch { unsigned chunkX,chunkY;bool roof=false,additive=false;std::vector<mu::assets::Vec3> positions;std::vector<std::array<float,2>> uv;std::shared_ptr<const mu::assets::Image> image; };
struct TerrainBatch { unsigned chunkX=0,chunkY=0;unsigned texture=0;bool overlay=false;std::vector<mu::assets::Vec3> positions;std::vector<std::array<float,2>> uv;std::vector<std::array<float,4>> color;std::shared_ptr<const mu::assets::Image> image; };
struct AnimatedEnvironment { mu::assets::WorldObject placement;std::shared_ptr<const ActorVisual> visual; };
struct WorldAssets { std::shared_ptr<const CharacterBody> body;std::filesystem::path dataDirectory; std::vector<AnimatedEnvironment> animatedEnvironment; CharacterResources characterResources; mu::assets::Image cursor,cursorTalk,cursorAttack,spiritSprite,levelFlare,levelGround,energySprite,sparkSprite;std::shared_ptr<const ActorVisual> fireSpell,spiritHead; ActorCatalog actors; std::vector<TerrainBatch> ground;std::array<unsigned,3> clipPeriods{}; mu::assets::Terrain terrain;mu::assets::ObjectFile objectFile;std::vector<EnvironmentBatch> environment;std::vector<unsigned> unmappedObjectTypes;std::vector<std::string> resourceWarnings;unsigned renderedObjects=0;std::vector<Part> parts;std::vector<mu::assets::Mesh> shadowMeshes;std::vector<std::vector<mu::assets::Bone>> idlePoses,walkPoses,runPoses; };
struct SceneData { double movementRate=1.;std::string mapName;std::shared_ptr<const WorldAssets> assets;unsigned x=135,y=124,direction=0; };
void verify_geometry_contract();
std::shared_ptr<SceneData> load(const std::string& assetRoot,unsigned map,unsigned x,unsigned y,unsigned direction,unsigned baseClass=0,const std::function<void(unsigned,unsigned)>& progress={},const CharacterResources* character=nullptr);
}
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <SceneKit/SceneKit.h>
#import <simd/simd.h>
#import <Metal/Metal.h>
#include <atomic>
#include <mutex>
#include <fstream>
namespace mu::viewer { struct JointPose { simd_float3 position;simd_quatf orientation; };struct EquipmentAnimation {std::array<std::vector<std::vector<JointPose>>,6> samples;std::array<unsigned,6> periods{},actions{};}; }
@class MuViewportRenderer,MuCharacterRig;
@interface MuWorldViewer:NSObject<NSWindowDelegate,SCNSceneRendererDelegate>{ SCNVector3 cameraHome, cameraTarget;double stepDistance;BOOL stepPlaced;SCNVector3 stepPosition;NSSound* levelSound;NSSound* evilSound; std::shared_ptr<mu::viewer::SceneData> sceneData; unsigned tileX,tileY; BOOL walking,assetsPrepared; NSArray<SCNNode*>* skeletonNodes; SCNNode* rigNode;SCNNode* shadowNode;unsigned activeClip;mu::viewer::RunPolicy runPolicy;std::atomic_bool runEligible,running;
 std::uint64_t lootRevision;unsigned lootGeneration,evilSequence;std::uint64_t castEffectRevision;std::map<unsigned,std::uint64_t> lootPending;NSMutableDictionary* lootNodes;std::shared_ptr<const mu::client::ViewportSnapshot> lootSnapshot;
 std::mutex effectMutex;NSMutableArray* effectTicks;NSMutableArray* activeEffects;
 std::mutex equipmentMutex;std::shared_ptr<const mu::viewer::CharacterBody> pendingEquipment,equipmentBody;NSArray<SCNNode*>* pendingEquipmentNodes;NSArray<MuCharacterRig*>* pendingEquipmentRigs;NSArray<MuCharacterRig*>* equipmentRigs;NSArray<SCNNode*>* playerMeshNodes;std::atomic_uint equipmentGeneration;std::atomic_uint equipmentApplied;unsigned pendingEquipmentGeneration;std::shared_ptr<const mu::viewer::EquipmentAnimation> pendingEquipmentAnimation,equipmentAnimation;BOOL combatStance;std::atomic_uint pendingSpell;double spellRemaining;unsigned spellVariant;
 unsigned interfaceColumns;MuViewportRenderer* viewportRenderer;std::mutex movementMutex;mu::viewer::MotionTrack motion;
 std::vector<std::vector<mu::viewer::JointPose>> idleSamples,walkSamples,runSamples;
 std::vector<mu::viewer::JointPose> currentJoints,blendJoints,appliedJoints;std::vector<simd_float4x4> jointMatrices;
 NSArray<SCNGeometry*>* shadowGeometries;NSArray<id<MTLBuffer>>* shadowBuffers;unsigned shadowFrame;NSArray<MuCharacterRig*>* environmentRigs;double environmentTime;NSArray<SCNNode*>* environmentNodes;NSArray<SCNNode*>* environmentChunks;int environmentFocusX,environmentFocusY;
 std::vector<unsigned> jointOrder;double clipTime,blendTime,lastFrameTime;unsigned perfFrames,perfLong;double perfMax;std::vector<double> perfIntervals;std::shared_ptr<std::ofstream> perfLog;dispatch_queue_t perfQueue;
 }
@property(nonatomic,strong) NSWindow* window;
@property(nonatomic,strong) SCNView* sceneView;
@property(nonatomic,strong) NSView* interfaceView;
- (void)playLevelUp;
- (void)setCastRevision:(std::uint64_t)revision;
- (void)playSkillEffect:(unsigned)skill target:(unsigned)target viewport:(std::shared_ptr<const mu::client::ViewportSnapshot>)snapshot;
- (void)setInterfaceColumns:(unsigned)columns;
- (unsigned)equipmentRevision;
- (unsigned)playerAction;
- (void)updateEquipment:(std::shared_ptr<const mu::viewer::CharacterBody>)body;
@property(nonatomic,strong) SCNNode* cameraNode;
@property(nonatomic,strong) SCNNode* actorNode;
@property(nonatomic,strong) NSTextField* infoLabel;
@property(nonatomic,copy) void (^onMove)(unsigned);
@property(nonatomic,copy) void (^onEntity)(unsigned);
@property(nonatomic,copy) void (^onSkillTarget)(unsigned);
@property(nonatomic,copy) BOOL (^onAreaSkill)(void);
@property(nonatomic,copy) void (^onLoot)(unsigned);
@property(nonatomic,copy) void (^onDestination)(unsigned,unsigned);
@property(nonatomic,copy) void (^onClose)(void);
- (void)updateViewport:(std::shared_ptr<const mu::client::ViewportSnapshot>)snapshot;
- (void)updateX:(unsigned)x y:(unsigned)y direction:(unsigned)direction;
- (NSInteger)heldDirection;
- (void)stopInput;
- (BOOL)pathPending;
- (BOOL)pathNearEnd;
- (std::pair<unsigned,unsigned>)movementEndpoint;
- (std::pair<unsigned,unsigned>)movementTile;
- (void)correctX:(unsigned)x y:(unsigned)y direction:(unsigned)direction;
- (void)confirmedPathX:(unsigned)x y:(unsigned)y directions:(const std::vector<unsigned>&)directions;
- (void)faceDirection:(unsigned)direction;
- (void)playSpell:(unsigned)skill;
- (void)setBootsLevel:(int)level;
- (BOOL)isRunning;
- (void)setMessage:(NSString*)message;
- (void)showScene:(std::shared_ptr<mu::viewer::SceneData>)data;
- (void)showScene:(std::shared_ptr<mu::viewer::SceneData>)data inWindow:(NSWindow*)window;
- (void)stop;
@end
#endif
