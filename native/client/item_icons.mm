#include "item_upgrade.hpp"
#include "item_models.hpp"
#include "item_icons.hpp"
#include "item_catalog.hpp"
#include "asset_images.hpp"
#import <SceneKit/SceneKit.h>
#import <Metal/Metal.h>
#include <mutex>
#include <cmath>
#include <limits>
#include <set>
#include <cstdio>
#include <stdexcept>
#include <algorithm>
namespace mu::ui {namespace {
std::mutex iconMutex;std::set<std::string> failures;
NSMutableDictionary<NSString*,SCNScene*>* scenes=[NSMutableDictionary dictionary];
NSMutableDictionary<NSString*,NSImage*>* cache=[NSMutableDictionary dictionary];
NSString* key(const std::string& root,const std::string& language,const mu::client::Item& item){return [NSString stringWithFormat:@"%s/%s/%u/%u",root.c_str(),language.c_str(),item.index,item.level];}
// ZzzObject::RenderPartObject tier passes, using original Effect textures.
SCNScene* makeScene(const std::string& root,const std::string& language,const mu::client::Item& item){auto data=std::filesystem::path(root)/"Data",path=mu::viewer::item_model_path(data,item);if(path.empty())return nil;auto model=mu::assets::load_model(path);if(item.index/512>=7&&item.index/512<=11){static std::vector<mu::assets::Bone> inventoryPose=mu::assets::load_model(data/"Player/Player.bmd",{0}).bones;model.bones=inventoryPose;}auto catalog=load_item_catalog(data,language);auto& attr=catalog->at(item.index);if(!attr.width||!attr.height)return nil;SCNScene* scene=[SCNScene scene];scene.background.contents=NSColor.clearColor;auto object=[SCNNode node];object.name=@"item";[scene.rootNode addChildNode:object];float low[3]={INFINITY,INFINITY,INFINITY},high[3]={-INFINITY,-INFINITY,-INFINITY};float ax=-M_PI/2,ay=0,az=0;if(item.index==13*512+20&&item.level){ax=255*M_PI/180;ay=160*M_PI/180;}if(item.index/512<=5){ax=M_PI;ay=3*M_PI/2;az=15*M_PI/180;}
 auto rotate=[&](mu::assets::Vec3 p){float x=p.x,y=std::cos(ax)*p.y-std::sin(ax)*p.z,z=std::sin(ax)*p.y+std::cos(ax)*p.z;float nx=std::cos(ay)*x+std::sin(ay)*z,nz=-std::sin(ay)*x+std::cos(ay)*z;return SCNVector3Make(std::cos(az)*nx-std::sin(az)*y,std::sin(az)*nx+std::cos(az)*y,nz);};
 for(auto& mesh:model.meshes){auto posed=mu::assets::posed_vertices(mesh,model.bones);mu::assets::Mesh nm;nm.vertices=mesh.normals;auto transformedNormals=mu::assets::posed_vertices(nm,model.bones);for(auto& v:nm.vertices)v.position={0,0,0};auto origins=mu::assets::posed_vertices(nm,model.bones);std::vector<SCNVector3> points,normals;std::vector<CGPoint> uv;std::vector<std::uint32_t> indices;for(auto& face:mesh.faces)for(unsigned corner=0;corner<3;corner++){auto p=rotate(posed.at(face.vertex[corner]));points.push_back(p);auto ni=face.normal[corner];auto n=transformedNormals.at(ni),o=origins.at(ni);auto normal=rotate({n.x-o.x,n.y-o.y,n.z-o.z});normals.push_back(normal);float values[3]={float(p.x),float(p.y),float(p.z)};for(unsigned axis=0;axis<3;axis++){low[axis]=std::min(low[axis],values[axis]);high[axis]=std::max(high[axis],values[axis]);}auto coord=mesh.uv.at(face.uv[corner]);uv.push_back(CGPointMake(coord[0],1-coord[1]));indices.push_back(indices.size());}
  auto vertices=[SCNGeometrySource geometrySourceWithVertices:points.data() count:points.size()];auto coords=[SCNGeometrySource geometrySourceWithTextureCoordinates:uv.data() count:uv.size()];auto faces=[SCNGeometryElement geometryElementWithData:[NSData dataWithBytes:indices.data() length:indices.size()*4] primitiveType:SCNGeometryPrimitiveTypeTriangles primitiveCount:indices.size()/3 bytesPerIndex:4];auto geometry=[SCNGeometry geometryWithSources:@[vertices,coords,[SCNGeometrySource geometrySourceWithNormals:normals.data() count:normals.size()]] elements:@[faces]];auto material=[SCNMaterial material];auto textureDirectory=item.index==13*512+29?data/"Npc":path.parent_path();auto image=mu::viewer::load_interface_image(textureDirectory,mesh.texture.c_str());material.diffuse.contents=image;material.lightingModelName=SCNLightingModelConstant;material.doubleSided=YES;material.blendMode=SCNBlendModeAlpha;geometry.materials=@[material];[object addChildNode:[SCNNode nodeWithGeometry:geometry]];mu::viewer::apply_item_upgrade(object,geometry,material,data,item);
 }
 if(!std::isfinite(low[0]))return nil;auto camera=[SCNNode node];camera.camera=[SCNCamera camera];camera.camera.usesOrthographicProjection=YES;double aspect=double(attr.width)/attr.height,span=std::max(double(high[1]-low[1]),std::hypot(high[0]-low[0],high[2]-low[2])/aspect)*1.12;camera.camera.orthographicScale=std::max(.1,span/2);float distance=std::max(high[2]-low[2],float(span))*3+10;camera.position=SCNVector3Make((low[0]+high[0])*.5,(low[1]+high[1])*.5,high[2]+distance);camera.camera.zNear=.1;camera.camera.zFar=distance*4+1000;[scene.rootNode addChildNode:camera];object.pivot=SCNMatrix4MakeTranslation((low[0]+high[0])*.5,(low[1]+high[1])*.5,(low[2]+high[2])*.5);object.position=SCNVector3Make((low[0]+high[0])*.5,(low[1]+high[1])*.5,(low[2]+high[2])*.5);return scene;
}
}
void prepare_item_icons(const std::string& root,const std::string& language,const mu::client::ItemSlots& items){std::lock_guard<std::mutex> guard(iconMutex);for(auto& pair:items){NSString* id=key(root,language,pair.second);if(cache[id]||failures.count(id.UTF8String))continue;try{auto scene=makeScene(root,language,pair.second);if(scene){scenes[id]=scene;auto attr=load_item_catalog(std::filesystem::path(root)/"Data",language)->at(pair.second.index);auto renderer=[SCNRenderer rendererWithDevice:MTLCreateSystemDefaultDevice() options:nil];renderer.scene=scene;renderer.pointOfView=scene.rootNode.childNodes.lastObject;cache[id]=[renderer snapshotAtTime:0 withSize:CGSizeMake(attr.width*64,attr.height*64) antialiasingMode:SCNAntialiasingModeMultisampling2X];}else failures.insert(id.UTF8String);}catch(const std::exception& e){failures.insert(id.UTF8String);std::fprintf(stderr,"[ItemIcon] %u:%u: %s\n",pair.second.index/512,pair.second.index%512,e.what());}}}
SCNScene* cached_item_scene(const std::string& root,const std::string& language,const mu::client::Item& item){std::lock_guard<std::mutex> guard(iconMutex);auto source=scenes[key(root,language,item)];if(!source)return nil;auto scene=[SCNScene scene];for(SCNNode* child in source.rootNode.childNodes)[scene.rootNode addChildNode:[child clone]];return scene;}
NSImage* cached_item_icon(const std::string& root,const std::string& language,const mu::client::Item& item){std::lock_guard<std::mutex> guard(iconMutex);return cache[key(root,language,item)];}
}

@implementation MuHoveredItemView{NSString* currentKey;}
- (BOOL)isOpaque{return NO;}
- (NSView*)hitTest:(NSPoint)point{return nil;}
- (void)showItem:(const mu::client::Item&)item root:(const std::string&)root language:(const std::string&)language rect:(NSRect)rect inView:(NSView*)host{
 NSString* id=[NSString stringWithFormat:@"%s/%s/%u/%u",root.c_str(),language.c_str(),item.index,item.level];self.frame=rect;if(![id isEqualToString:currentKey]||self.superview!=host){self.scene=mu::ui::cached_item_scene(root,language,item);currentKey=id;if(!self.scene){[self hide];return;}self.backgroundColor=NSColor.clearColor;self.preferredFramesPerSecond=60;self.antialiasingMode=SCNAntialiasingModeMultisampling2X;self.pointOfView=self.scene.rootNode.childNodes.lastObject;auto node=[self.scene.rootNode childNodeWithName:@"item" recursively:NO];float az=item.index/512<=5?15*M_PI/180:0;auto action=[SCNAction rotateByAngle:2*M_PI aroundAxis:SCNVector3Make(-sin(az),cos(az),0) duration:.8];[node runAction:[SCNAction repeatActionForever:action]];[host addSubview:self];}self.playing=YES;
}
- (void)setRotating:(BOOL)rotating{auto node=[self.scene.rootNode childNodeWithName:@"item" recursively:NO];[node removeAllActions];node.rotation=SCNVector4Make(0,1,0,0);if(rotating){auto action=[SCNAction rotateByAngle:2*M_PI aroundAxis:SCNVector3Make(0,1,0) duration:.8];[node runAction:[SCNAction repeatActionForever:action]];}self.playing=YES;}
- (void)hide{self.playing=NO;self.scene=nil;currentKey=nil;[self removeFromSuperview];}
@end
@implementation MuItemEffectsLayer{NSMutableDictionary<NSNumber*,SCNNode*>* itemsBySlot;}
- (BOOL)isOpaque{return NO;}
- (NSView*)hitTest:(NSPoint)point{return nil;}
- (void)updateItems:(const std::vector<mu::ui::ItemPresentation>&)items root:(const std::string&)root language:(const std::string&)language{
 self.playing=NO;self.scene=[SCNScene scene];itemsBySlot=[NSMutableDictionary dictionary];self.backgroundColor=NSColor.clearColor;self.preferredFramesPerSecond=60;self.antialiasingMode=SCNAntialiasingModeMultisampling2X;auto camera=[SCNNode node];camera.camera=[SCNCamera camera];camera.camera.usesOrthographicProjection=YES;camera.camera.orthographicScale=self.bounds.size.height/2;camera.camera.zNear=1;camera.camera.zFar=20000;camera.position=SCNVector3Make(self.bounds.size.width/2,self.bounds.size.height/2,10000);[self.scene.rootNode addChildNode:camera];self.pointOfView=camera;
 for(const auto& entry:items){if(entry.item.level<7||entry.item.index/512>11)continue;auto source=mu::ui::cached_item_scene(root,language,entry.item);if(!source)continue;auto model=[source.rootNode childNodeWithName:@"item" recursively:NO];auto lens=source.rootNode.childNodes.lastObject.camera;CGFloat factor=entry.rect.size.height/(2*lens.orthographicScale);auto group=[SCNNode node];group.scale=SCNVector3Make(factor,factor,factor);group.position=SCNVector3Make(NSMidX(entry.rect)-model.position.x*factor,self.bounds.size.height-NSMidY(entry.rect)-model.position.y*factor,-model.position.z*factor);[group addChildNode:model];[self.scene.rootNode addChildNode:group];itemsBySlot[@(entry.slot)]=group;}
 self.playing=itemsBySlot.count>0;
}
- (BOOL)rendersSlot:(unsigned)slot{return itemsBySlot[@(slot)]!=nil;}
- (void)hideSlot:(NSNumber*)slot{for(NSNumber* key in itemsBySlot)itemsBySlot[key].hidden=slot&&[key isEqualToNumber:slot];}
- (void)stop{self.playing=NO;self.scene=nil;itemsBySlot=nil;[self removeFromSuperview];}
@end
