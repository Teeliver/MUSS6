#include "lobby_scene.hpp"
#include "asset_images.hpp"
#include "entity_view.hpp"
#import <simd/simd.h>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <cmath>
#include <set>
#include <tuple>
#include <stdexcept>
namespace mu::lobby {
std::shared_ptr<const Scene> load(const std::string& root,const std::function<void(unsigned,unsigned)>& progress,unsigned world){
 if(world!=74&&world!=75)throw std::runtime_error("Unsupported lobby scene world");auto out=std::make_shared<Scene>();out->world=world;auto data=std::filesystem::path(root)/"Data";
 if(world==75)out->selectionImage=std::make_shared<mu::assets::Image>(mu::viewer::decode_texture(mu::viewer::resolve_model_texture(data/"Skill","gmmzine.jpg")));
 // Reference branch PJH_NEW_SERVER_SELECT_MAP selects World 73 (folder 74).
 // Keep this loader isolated from Lorencia's map/material rules.
 out->placements=mu::assets::load_world_objects(data,world-1);std::set<unsigned> types;for(auto& o:out->placements.objects)types.insert(o.type);
 auto directory=data/("World"+std::to_string(world));
 if(world==74){auto camera=mu::assets::read_file(directory/"CWScript74.cws");
 auto u32=[&](unsigned p){if(p+4>camera.size())throw std::runtime_error("Truncated original lobby camera script");unsigned n=0;for(unsigned i=0;i<4;i++)n|=unsigned(camera[p+i])<<(8*i);return n;};
 auto number=[&](unsigned p){auto n=u32(p);float f;std::memcpy(&f,&n,4);if(!std::isfinite(f)||std::abs(f)>100000)throw std::runtime_error("Invalid lobby camera coordinate");return f;};
 if(u32(0)!=0x00535743||u32(4)<2||u32(4)>1000||camera.size()!=8+u32(4)*28)throw std::runtime_error("Unsupported Windows CWS layout");
 for(unsigned i=0;i<u32(4);i++){unsigned p=8+i*28;Waypoint w{number(p+4),number(p+8),number(p+12),u32(p+16),number(p+20),number(p+24)};if(w.speed<=0||w.distance<=0||w.delay>60000)throw std::runtime_error("Invalid original camera timing");out->camera.push_back(w);}
 }
 auto terrain=mu::assets::load_terrain(data,world-1);out->terrain=terrain;auto light=mu::viewer::decode_texture(directory/"TerrainLight.OZJ");if(light.width!=256||light.height!=256)throw std::runtime_error("Invalid lobby light map");
 const char* tileNames[]={"TileGrass01.jpg","TileGrass02.jpg","TileGround01.jpg","TileGround02.jpg","TileGround03.jpg","TileWater01.jpg","TileWood01.jpg","TileRock01.jpg","TileRock02.jpg","TileRock03.jpg","AlphaTile01.tga","TileRock05.jpg","TileRock06.jpg","TileRock07.jpg"};
 std::map<unsigned,std::shared_ptr<const mu::assets::Image>> tiles;std::map<std::tuple<unsigned,bool,unsigned,unsigned>,unsigned> groups;
 auto emit=[&](unsigned x,unsigned y,unsigned texture,bool overlay){if(texture==255)return;if(!tiles.count(texture))tiles[texture]=std::make_shared<mu::assets::Image>(mu::viewer::decode_texture(mu::viewer::resolve_model_texture(directory,tileNames[texture])));auto key=std::make_tuple(texture,overlay,x/16,y/16);auto i=groups.find(key);if(i==groups.end()){Ground g;g.image=tiles.at(texture);g.overlay=overlay;i=groups.emplace(key,out->ground.size()).first;out->ground.push_back(std::move(g));}auto& g=out->ground[i->second];constexpr unsigned corners[6][2]={{0,0},{1,1},{1,0},{0,0},{0,1},{1,1}};for(auto& c:corners){unsigned vx=x+c[0],vy=y+c[1],vi=vy*256+vx;g.positions.push_back({float(vx*100),float(vy*100),terrain.height(vx,vy)*100+(overlay?.05f:0.f)});g.uv.push_back({vx*64.f/g.image->width,vy*64.f/g.image->height});g.colors.push_back({light.rgba[vi*4]/255.f,light.rgba[vi*4+1]/255.f,light.rgba[vi*4+2]/255.f,overlay?terrain.alpha[vi]/255.f:1.f});}};
 for(unsigned y=0;y<255;y++)for(unsigned x=0;x<255;x++){unsigned i=y*256+x;if(terrain.attributes[i]&8)continue;bool full=terrain.layer2[i]!=255&&terrain.alpha[i]==255&&terrain.alpha[i+1]==255&&terrain.alpha[i+256]==255&&terrain.alpha[i+257]==255;emit(x,y,full?terrain.layer2[i]:terrain.layer1[i],false);if(!full&&(terrain.alpha[i]||terrain.alpha[i+1]||terrain.alpha[i+256]||terrain.alpha[i+257]))emit(x,y,terrain.layer2[i],true);}
 std::map<std::string,std::shared_ptr<const mu::assets::Image>> textures;unsigned completed=0;if(progress)progress(0,types.size());
 for(auto type:types){char name[64];snprintf(name,sizeof(name),"Object%02u.bmd",type+1);auto path=data/("Object"+std::to_string(world))/name;if(!std::filesystem::exists(path)){out->missingModelTypes.push_back(type);if(progress)progress(++completed,types.size());continue;}auto model=mu::assets::load_model(path);Model visual;
  for(auto& mesh:model.meshes){auto found=textures.find(mesh.texture);if(found==textures.end())found=textures.emplace(mesh.texture,std::make_shared<mu::assets::Image>(mu::viewer::decode_texture(mu::viewer::resolve_model_texture(data/("Object"+std::to_string(world)),mesh.texture)))).first;
   auto posed=mu::assets::posed_vertices(mesh,model.bones);out->triangles+=mesh.faces.size();visual.parts.push_back({std::move(mesh),std::move(posed),found->second});}
  out->models.emplace(type,std::move(visual));if(progress)progress(++completed,types.size());
 }
 return out;
}
}
namespace {
NSImage* image(const mu::assets::Image& a){auto color=CGColorSpaceCreateDeviceRGB();auto provider=CGDataProviderCreateWithData(nullptr,a.rgba.data(),a.rgba.size(),nullptr);auto cg=CGImageCreate(a.width,a.height,8,32,a.width*4,color,kCGImageAlphaLast|kCGBitmapByteOrder32Big,provider,nullptr,YES,kCGRenderingIntentDefault);auto out=[[NSImage alloc] initWithCGImage:cg size:NSMakeSize(a.width,a.height)];CGImageRelease(cg);CGDataProviderRelease(provider);CGColorSpaceRelease(color);return out;}
SCNNode* groundNode(const mu::lobby::Ground& batch, NSMutableDictionary* textures){std::vector<SCNVector3> p;std::vector<CGPoint> uv;std::vector<unsigned> idx;for(unsigned i=0;i<batch.positions.size();i++){auto v=batch.positions[i];p.push_back(SCNVector3Make(v.x,v.y,v.z));uv.push_back(CGPointMake(batch.uv[i][0],1-batch.uv[i][1]));idx.push_back(i);}auto vertices=[SCNGeometrySource geometrySourceWithVertices:p.data() count:p.size()];auto coords=[SCNGeometrySource geometrySourceWithTextureCoordinates:uv.data() count:uv.size()];auto colors=[SCNGeometrySource geometrySourceWithData:[NSData dataWithBytes:batch.colors.data() length:batch.colors.size()*16] semantic:SCNGeometrySourceSemanticColor vectorCount:batch.colors.size() floatComponents:YES componentsPerVector:4 bytesPerComponent:4 dataOffset:0 dataStride:16];auto element=[SCNGeometryElement geometryElementWithData:[NSData dataWithBytes:idx.data() length:idx.size()*4] primitiveType:SCNGeometryPrimitiveTypeTriangles primitiveCount:idx.size()/3 bytesPerIndex:4];auto geo=[SCNGeometry geometryWithSources:@[vertices,coords,colors] elements:@[element]];auto material=[SCNMaterial material];auto key=[NSValue valueWithPointer:batch.image.get()];NSImage* texture=textures[key];if(!texture){texture=image(*batch.image);textures[key]=texture;}material.diffuse.contents=texture;material.diffuse.wrapS=SCNWrapModeRepeat;material.diffuse.wrapT=SCNWrapModeRepeat;material.lightingModelName=SCNLightingModelConstant;material.doubleSided=YES;material.blendMode=batch.overlay?SCNBlendModeAlpha:SCNBlendModeReplace;material.writesToDepthBuffer=!batch.overlay;geo.materials=@[material];auto node=[SCNNode nodeWithGeometry:geo];node.renderingOrder=batch.overlay?-1:-2;return node;}
SCNNode* modelNode(const mu::lobby::Model& model){auto root=[SCNNode node];for(auto& part:model.parts){std::vector<SCNVector3> positions;std::vector<CGPoint> uv;std::vector<unsigned> faces;
 for(auto& face:part.mesh.faces)for(unsigned k=0;k<3;k++){auto p=part.posed.at(face.vertex[k]);auto t=part.mesh.uv.at(face.uv[k]);positions.push_back(SCNVector3Make(p.x,p.y,p.z));uv.push_back(CGPointMake(t[0],1-t[1]));faces.push_back(faces.size());}
 auto vertices=[SCNGeometrySource geometrySourceWithVertices:positions.data() count:positions.size()];auto coords=[SCNGeometrySource geometrySourceWithTextureCoordinates:uv.data() count:uv.size()];auto indices=[SCNGeometryElement geometryElementWithData:[NSData dataWithBytes:faces.data() length:faces.size()*4] primitiveType:SCNGeometryPrimitiveTypeTriangles primitiveCount:faces.size()/3 bytesPerIndex:4];auto geo=[SCNGeometry geometryWithSources:@[vertices,coords] elements:@[indices]];
 auto material=[SCNMaterial material];auto texture=image(*part.image);material.diffuse.contents=texture;material.diffuse.wrapS=SCNWrapModeRepeat;material.diffuse.wrapT=SCNWrapModeRepeat;material.lightingModelName=SCNLightingModelConstant;material.doubleSided=YES;
 bool alpha=false;for(std::size_t i=3;i<part.image->rgba.size();i+=4)if(part.image->rgba[i]<250){alpha=true;break;}if(alpha){material.transparent.contents=texture;material.transparencyMode=SCNTransparencyModeAOne;material.blendMode=SCNBlendModeAlpha;material.shaderModifiers=@{SCNShaderModifierEntryPointFragment:@"if (_output.color.a < 0.25) discard_fragment();"};}geo.materials=@[material];[root addChildNode:[SCNNode nodeWithGeometry:geo]];
 }return root;}
}
@implementation MuLobbyScene {
 std::shared_ptr<const mu::lobby::Scene> _data;SCNNode* _camera;unsigned _target;double _last,_accumulator;float _x,_y,_yaw,_distance,_beforeX,_beforeY,_beforeYaw,_beforeDistance;unsigned _delay;BOOL _active,_prepared;std::atomic_bool _resetClock;id _occlusionObserver;NSMutableArray<MuCharacterRig*>* _characters;double _animationTime;std::mutex _charactersMutex;std::atomic<int> _selectedRow;std::atomic<int> _appliedSelection;SCNNode* _selectionRoot;
}
- (NSView*)hitTest:(NSPoint)point{return _data->world==75?[super hitTest:point]:nil;}
- (NSInteger)characterRowAtPoint:(NSPoint)point{
 // Pick a projected character volume. Do not ask SceneKit to CPU-skin the
 // animated render mesh during hit testing while its joints are advancing.
 NSArray<MuCharacterRig*>* rigs;{std::lock_guard<std::mutex> lock(_charactersMutex);rigs=_characters;}NSInteger selected=-1;CGFloat depth=INFINITY;
 for(unsigned i=0;i<rigs.count;i++){auto p=rigs[i].node.position;CGFloat minX=INFINITY,minY=INFINITY,maxX=-INFINITY,maxY=-INFINITY,z=1;for(float x:{-60.f,60.f})for(float y:{-60.f,60.f})for(float h:{0.f,220.f}){auto screen=[self projectPoint:SCNVector3Make(p.x+x,p.y+y,p.z+h)];minX=std::min(minX,screen.x);maxX=std::max(maxX,screen.x);minY=std::min(minY,screen.y);maxY=std::max(maxY,screen.y);z=std::min(z,screen.z);}if(z>=0&&z<depth&&NSPointInRect(point,NSMakeRect(minX,minY,maxX-minX,maxY-minY))){selected=i;depth=z;}}
 return selected;
}
- (void)mouseDown:(NSEvent*)event{if(_data->world!=75)return;NSInteger row=[self characterRowAtPoint:[self convertPoint:event.locationInWindow fromView:nil]];if(row>=0&&self.onCharacter)self.onCharacter(unsigned(row),event.clickCount>1);}
- (void)selectCharacter:(NSInteger)row{_selectedRow.store(int(row));}
- (NSPoint)characterAnchor:(unsigned)row{MuCharacterRig* rig;{std::lock_guard<std::mutex> lock(_charactersMutex);if(row>=_characters.count)return NSZeroPoint;rig=_characters[row];}auto p=rig.node.position;p.z+=350;auto projected=[self projectPoint:p];return NSMakePoint(projected.x,projected.y);}
- (void)setCharacters:(const std::vector<mu::lobby::CharacterVisual>&)characters{
 _selectedRow.store(-1);NSMutableArray<MuCharacterRig*>* next=[NSMutableArray array];constexpr float slots[5][3]={{8008,18885,115},{7986,19145,90},{8046,19400,75},{8133,19645,60},{8282,19845,35}};
 for(unsigned i=0;i<characters.size();i++){const auto& entry=characters[i];auto rig=[[MuCharacterRig alloc] initWithVisual:entry.visual];auto p=slots[entry.character.slot];rig.node.position=SCNVector3Make(p[0],p[1],_data->terrain.heightAt(p[0]/100.,p[1]/100.)*100);rig.node.eulerAngles=SCNVector3Make(0,0,p[2]*M_PI/180);rig.node.name=[NSString stringWithFormat:@"character-row:%u",i];[next addObject:rig];}
 NSArray<MuCharacterRig*>* previous;{std::lock_guard<std::mutex> lock(_charactersMutex);previous=_characters;_characters=next;}_appliedSelection=-2;for(MuCharacterRig* rig in previous)[rig.node removeFromParentNode];for(MuCharacterRig* rig in next)[self.scene.rootNode addChildNode:rig.node];
}

- (instancetype)initWithData:(std::shared_ptr<const mu::lobby::Scene>)data{
 if((self=[super initWithFrame:NSMakeRect(0,0,800,600) options:@{SCNViewOptionPreferredRenderingAPI:@(SCNRenderingAPIMetal)}])){
  _resetClock.store(false);_selectedRow.store(-1);_appliedSelection=-2;_data=std::move(data);self.scene=[SCNScene scene];NSMutableDictionary* textures=[NSMutableDictionary dictionary];for(auto& batch:_data->ground)[self.scene.rootNode addChildNode:groundNode(batch,textures)];self.scene.background.contents=NSColor.blackColor;NSMutableDictionary<NSNumber*,SCNNode*>* models=[NSMutableDictionary dictionary];
  for(auto& pair:_data->models)models[@(pair.first)]=modelNode(pair.second);
  const std::set<unsigned> effectMarkers={79,80,82,83,84,85,86,129,130,131,132};for(auto& object:_data->placements.objects){if(effectMarkers.count(object.type)||!_data->models.count(object.type))continue;auto node=[models[@(object.type)] clone];node.position=SCNVector3Make(object.position.x,object.position.y,object.position.z);node.scale=SCNVector3Make(object.scale,object.scale,object.scale);auto r=object.rotation;float d=M_PI/180;node.simdOrientation=simd_mul(simd_quaternion(r.z*d,simd_make_float3(0,0,1)),simd_mul(simd_quaternion(r.y*d,simd_make_float3(0,1,0)),simd_quaternion(r.x*d,simd_make_float3(1,0,0))));[self.scene.rootNode addChildNode:node];}
  _camera=[SCNNode node];_camera.camera=[SCNCamera camera];_camera.camera.fieldOfView=65;_camera.camera.zNear=20;[self.scene.rootNode addChildNode:_camera];self.pointOfView=_camera;self.allowsCameraControl=NO;self.preferredFramesPerSecond=60;self.antialiasingMode=SCNAntialiasingModeMultisampling2X;self.delegate=self;
  if(_data->world==75){_selectionRoot=[SCNNode node];_selectionRoot.name=@"source-selected-hero-aurora";_selectionRoot.hidden=YES;[self.scene.rootNode addChildNode:_selectionRoot];auto texture=image(*_data->selectionImage);for(float size:{180.f,120.f}){auto plane=[SCNPlane planeWithWidth:size height:size];auto material=[SCNMaterial material];material.diffuse.contents=texture;material.lightingModelName=SCNLightingModelConstant;material.blendMode=SCNBlendModeAdd;material.writesToDepthBuffer=NO;material.doubleSided=YES;plane.materials=@[material];auto node=[SCNNode nodeWithGeometry:plane];node.renderingOrder=5;[_selectionRoot addChildNode:node];}}
  // CreateLogInScene SetTourMode(TRUE,FALSE,1): origin waypoint 0, target 1.
  if(_data->world==75){_camera.camera.fieldOfView=35;_camera.position=SCNVector3Make(9758.93,18913.11,675.5);float yaw=-75*M_PI/180,pitch=-84.5*M_PI/180;[_camera lookAt:SCNVector3Make(_camera.position.x-sin(yaw)*sin(pitch)*1000,_camera.position.y-cos(yaw)*sin(pitch)*1000,_camera.position.z-cos(pitch)*1000) up:SCNVector3Make(0,0,1) localFront:SCNVector3Make(0,0,-1)];_camera.camera.zFar=10000;}else{_target=1;_x=_data->camera[0].x;_y=_data->camera[0].y;_distance=_data->camera[0].distance;auto& next=_data->camera[1];_yaw=atan2(next.y-_y,next.x-_x);_beforeX=_x;_beforeY=_y;_beforeYaw=_yaw;_beforeDistance=_distance;[self placeCameraX:_x y:_y yaw:_yaw distance:_distance];}self.playing=NO;
  __weak MuLobbyScene* weak=self;[self prepareObjects:@[self.scene] withCompletionHandler:^(BOOL success){dispatch_async(dispatch_get_main_queue(),^{MuLobbyScene* owner=weak;if(owner){owner->_prepared=success;[owner setActive:owner->_active];}});}];
 }return self;
}
- (void)setActive:(BOOL)active{_active=active;_resetClock.store(true);self.playing=active&&_prepared&&(!self.window||(self.window.occlusionState&NSWindowOcclusionStateVisible));}
- (void)viewDidMoveToWindow{[super viewDidMoveToWindow];if(_occlusionObserver)[NSNotificationCenter.defaultCenter removeObserver:_occlusionObserver];_occlusionObserver=nil;if(self.window){__weak MuLobbyScene* weak=self;_occlusionObserver=[NSNotificationCenter.defaultCenter addObserverForName:NSWindowDidChangeOcclusionStateNotification object:self.window queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification* note){MuLobbyScene* owner=weak;if(owner)[owner setActive:owner->_active];}];}[self setActive:_active];}
- (void)dealloc{if(_occlusionObserver)[NSNotificationCenter.defaultCenter removeObserver:_occlusionObserver];}

- (void)placeCameraX:(float)x y:(float)y yaw:(float)yaw distance:(float)distance{
 // ZzzAI::CreateAngle(dx,-dy), AngleMatrix/VectorIRotate and the inverse
 // OpenGL view rotation are different matrix orders. Preserve both explicitly.
 float d=110*distance,a=float(M_PI/2)-yaw,s=sin(a),c=cos(a);
 _camera.position=SCNVector3Make(x-d*s,y+d*c*.3826834f,d-550-d*c*.9238795f);
 [_camera lookAt:SCNVector3Make(_camera.position.x+s*923.8795f,_camera.position.y+c*923.8795f,_camera.position.z+382.6834f) up:SCNVector3Make(0,0,1) localFront:SCNVector3Make(0,0,-1)];_camera.camera.zFar=390*distance*1.4f;
}
- (void)renderer:(id<SCNSceneRenderer>)renderer updateAtTime:(NSTimeInterval)time{
 if(_resetClock.exchange(false))_last=0;double dt=_last?std::clamp(time-_last,0.,.1):0;_last=time;if(_data->world==75){_animationTime+=dt;[SCNTransaction begin];[SCNTransaction setDisableActions:YES];NSArray<MuCharacterRig*>* rigs;{std::lock_guard<std::mutex> lock(_charactersMutex);rigs=_characters;}int selected=_selectedRow.load();_selectionRoot.hidden=selected<0||selected>=rigs.count;
 if(selected!=_appliedSelection){for(unsigned i=0;i<rigs.count;i++){BOOL chosen=int(i)==selected;rigs[i].node.opacity=1.;}_appliedSelection=selected;}
 if(!_selectionRoot.hidden){auto p=rigs[selected].node.position;p.z+=1;_selectionRoot.position=p;float luminance=sin(_animationTime*1.5)*.3+.5;unsigned i=0;for(SCNNode* layer in _selectionRoot.childNodes){layer.eulerAngles=SCNVector3Make(0,0,_animationTime*10*M_PI/180*(i++?-1:1));layer.geometry.firstMaterial.multiply.contents=[NSColor colorWithWhite:luminance alpha:1];}}
 for(MuCharacterRig* rig in rigs)[rig advance:_animationTime];[SCNTransaction commit];return;}_accumulator+=dt;
 // Preserve reference movement tick; render remains 60Hz. No disk access here.
 while(_accumulator>=.04){_accumulator-=.04;_beforeX=_x;_beforeY=_y;_beforeYaw=_yaw;_beforeDistance=_distance;auto& target=_data->camera[_target];if(_delay++<target.delay)continue;float dx=target.x-_x,dy=target.y-_y,length=hypot(dx,dy);
  if(length<=target.speed){_x=target.x;_y=target.y;_target=(_target+1)%_data->camera.size();_delay=0;continue;}
  _x+=dx/length*target.speed;_y+=dy/length*target.speed;float desired=atan2(dy,dx),difference=remainder(desired-_yaw,float(2*M_PI));_yaw+=std::clamp(difference/30,-float(M_PI/180),float(M_PI/180));_distance+=(target.distance-_distance)*.04f;
 }
 [SCNTransaction begin];[SCNTransaction setDisableActions:YES];float alpha=_accumulator/.04;[self placeCameraX:_beforeX+(_x-_beforeX)*alpha y:_beforeY+(_y-_beforeY)*alpha yaw:_beforeYaw+remainder(_yaw-_beforeYaw,float(2*M_PI))*alpha distance:_beforeDistance+(_distance-_beforeDistance)*alpha];[SCNTransaction commit];
}
@end
