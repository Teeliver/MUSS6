#include "item_upgrade.hpp"
#include "asset_images.hpp"
namespace mu::viewer {
void apply_item_upgrade(SCNNode* object,SCNGeometry* geometry,SCNMaterial* base,const std::filesystem::path& data,const mu::client::Item& item){unsigned level=item.level,group=item.index/512,type=item.index%512;if(group>11)return;float r=1,g=1,b=1;if(level>=3&&level<5){g=b=.6;}else if(level<7&&level>=5){r=.5;g=.7;}else if(level>=7){r=g=b=level<9?.8:.9;}base.multiply.contents=[NSColor colorWithRed:r green:g blue:b alpha:1];if(level<7)return;
 auto pass=[&](unsigned mode,NSColor* color){auto material=[SCNMaterial material];material.lightingModelName=SCNLightingModelConstant;material.diffuse.contents=mu::viewer::load_interface_image(data/"Effect",mode==0?"Chrome01.jpg":mode==1?"Shiny01.jpg":"Chrome02.jpg");material.diffuse.wrapS=mode==0?SCNWrapModeRepeat:SCNWrapModeClamp;material.diffuse.wrapT=material.diffuse.wrapS;material.diffuse.minificationFilter=mode>=2?SCNFilterModeNearest:SCNFilterModeLinear;material.diffuse.magnificationFilter=material.diffuse.minificationFilter;material.multiply.contents=color;material.blendMode=SCNBlendModeAdd;material.doubleSided=YES;material.writesToDepthBuffer=NO;NSString* uv;
 if(mode==0)uv=@"float w=fmod(scn_frame.time,10.0)*0.1;_geometry.texcoords[0]=float2(_geometry.normal.z*0.5+w,1.0-(_geometry.normal.y*0.5+w*2.0));";
 else if(mode==1)uv=@"_geometry.texcoords[0]=float2(_geometry.normal.z*0.5+0.2,1.0-(_geometry.normal.y*0.5+0.5));";
 else if(mode==2)uv=@"float w=fmod(scn_frame.time,5.0)*0.24-0.4;_geometry.texcoords[0]=float2((_geometry.normal.z+_geometry.normal.x)*0.8+w*2.0,1.0-((_geometry.normal.y+_geometry.normal.x)+w*3.0));";
 else uv=@"float w=fmod(scn_frame.time,10.0)*0.1;float3 l=float3(cos(scn_frame.time),sin(scn_frame.time*2.0),1.0);float d=dot(_geometry.normal,l);_geometry.texcoords[0]=float2(d+_geometry.normal.y*0.5+l.y*3.0,1.0-(1.0-d-_geometry.normal.z*0.5-w*3.0));";
 material.shaderModifiers=@{SCNShaderModifierEntryPointGeometry:[@"#pragma body\n" stringByAppendingString:uv]};SCNGeometry* overlay=[geometry copy];overlay.materials=@[material];auto node=[SCNNode nodeWithGeometry:overlay];node.renderingOrder=10+mode;[object addChildNode:node];};
 // Common material palette from PartObjectColor; explicit overrides for mapped models.
 NSColor* chrome=[NSColor colorWithRed:1 green:.5 blue:0 alpha:1];if((group==0&&type==14)||(group==5&&type==5))chrome=[NSColor colorWithRed:0 green:.5 blue:1 alpha:1];if((group==4&&(type==5||type==13))||(group==5&&type==9)||(group==0&&type==21))chrome=NSColor.whiteColor;
 pass(0,chrome);if(level>=9)pass(1,chrome);if(level>=11)pass(level<13?2:3,(group>=7&&(type==4||type==14||type==15))?[NSColor colorWithRed:1 green:.5 blue:0 alpha:1]:NSColor.whiteColor);
}
}
