#include "character_create.hpp"
#include "asset_images.hpp"
#include "entity_view.hpp"
#include "lobby_view.hpp"
#include "source_font.hpp"
#include <atomic>
#include <algorithm>
#import <simd/simd.h>
namespace mu::lobby {
std::shared_ptr<const CreationPreviews> load_creation_previews(const std::string& root){
 auto result=std::make_shared<CreationPreviews>();auto folder=std::filesystem::path(root)/"Data/Logo";std::map<std::string,std::shared_ptr<const mu::assets::Image>> textures;
 for(unsigned cls=0;cls<7;cls++){char filename[40];snprintf(filename,sizeof(filename),"NewFace%02u.bmd",cls+1);auto model=mu::assets::load_model(folder/filename,{0,1});for(unsigned clip=0;clip<2;clip++){auto v=std::make_shared<mu::viewer::ActorVisual>();v->bones=model.bones;v->idle=model.clips.at(clip);v->idlePeriod=std::max(1u,unsigned(v->idle.size())-(model.lockedPositions.at(clip)?1:0));v->scale=1;for(const auto& mesh:model.meshes){auto path=mu::viewer::resolve_model_texture(folder,mesh.texture);auto key=path.string();if(!textures.count(key))textures[key]=std::make_shared<mu::assets::Image>(mu::viewer::decode_texture(path));v->parts.push_back({mesh,textures.at(key)});}(*result)[cls][clip]=v;}}
 return result;
}
}
@interface MuCreationPreview:SCNView<SCNSceneRendererDelegate>
- (instancetype)initWithPreviews:(std::shared_ptr<const mu::lobby::CreationPreviews>)data;
- (void)choose:(unsigned)cls;
@end
@implementation MuCreationPreview {NSArray<NSArray<MuCharacterRig*>*>* _rigs;std::shared_ptr<const mu::lobby::CreationPreviews> _data;std::atomic<unsigned> _selection;unsigned _current;double _elapsed,_last;}
- (instancetype)initWithPreviews:(std::shared_ptr<const mu::lobby::CreationPreviews>)data{if((self=[super initWithFrame:NSMakeRect(0,0,410,335) options:@{SCNViewOptionPreferredRenderingAPI:@(SCNRenderingAPIMetal)}])){_data=data;_selection.store(1);_current=99;self.scene=[SCNScene scene];self.scene.background.contents=NSColor.clearColor;self.backgroundColor=NSColor.clearColor;self.preferredFramesPerSecond=60;self.antialiasingMode=SCNAntialiasingModeMultisampling2X;auto camera=[SCNNode node];camera.camera=[SCNCamera camera];camera.camera.fieldOfView=10;camera.camera.zNear=1;camera.camera.zFar=2000;camera.position=SCNVector3Make(10,-500,48);[camera lookAt:SCNVector3Make(10,0,48) up:SCNVector3Make(0,0,1) localFront:SCNVector3Make(0,0,-1)];[self.scene.rootNode addChildNode:camera];self.pointOfView=camera;NSMutableArray* rigs=[NSMutableArray array];constexpr float scale[7]={5.9,6.05,9.1,6,6,9.1,6};constexpr float angle[7][3]={{0,0,-40},{0,0,-12},{8,0,5},{8,0,-13},{8,0,-18},{2,0,2},{0,0,0}};constexpr float offset[7][3]={{0,0,0},{0,0,0},{4.8,0,0},{0,0,1.8},{0,0,0},{4.8,0,4},{9.8,0,-7.5}};for(unsigned c=0;c<7;c++){NSMutableArray* pair=[NSMutableArray array];for(unsigned clip=0;clip<2;clip++){auto rig=[[MuCharacterRig alloc] initWithVisual:(*data)[c][clip]];rig.node.scale=SCNVector3Make(scale[c],scale[c],scale[c]);auto a=angle[c];float d=M_PI/180;rig.node.simdOrientation=simd_mul(simd_quaternion(a[2]*d,simd_make_float3(0,0,1)),simd_mul(simd_quaternion(a[1]*d,simd_make_float3(0,1,0)),simd_quaternion(a[0]*d,simd_make_float3(1,0,0))));auto p=offset[c];rig.node.position=SCNVector3Make(p[0],p[1],p[2]);rig.node.hidden=YES;[self.scene.rootNode addChildNode:rig.node];[pair addObject:rig];}[rigs addObject:pair];}_rigs=rigs;self.delegate=self;self.playing=NO;}return self;}
- (void)choose:(unsigned)cls{_selection.store(cls);}
- (void)renderer:(id<SCNSceneRenderer>)renderer updateAtTime:(NSTimeInterval)t{unsigned cls=_selection.load();if(cls>=7)return;double dt=_last?std::clamp(t-_last,0.,.1):0;_last=t;if(cls!=_current){_current=cls;_elapsed=0;}else _elapsed+=dt;double speed=cls==5?.25:.3,intro=(*_data)[cls][1]->idlePeriod/(25*speed);unsigned clip=_elapsed<intro?1:0;[SCNTransaction begin];[SCNTransaction setDisableActions:YES];for(unsigned c=0;c<7;c++)for(unsigned k=0;k<2;k++)_rigs[c][k].node.hidden=c!=cls||k!=clip;[_rigs[cls][clip] advance:(_elapsed-(clip==0?intro:0))*speed/.25];[SCNTransaction commit];}
@end
@interface MuClassButton:NSButton
@property(nonatomic,strong) NSImage* sheet;
@property(nonatomic) BOOL hovered;
@property(nonatomic,strong) NSTrackingArea* tracking;
@end
@implementation MuClassButton
- (BOOL)isFlipped{return YES;}
- (void)updateTrackingAreas{[super updateTrackingAreas];if(self.tracking)[self removeTrackingArea:self.tracking];self.tracking=[[NSTrackingArea alloc] initWithRect:NSZeroRect options:NSTrackingMouseEnteredAndExited|NSTrackingActiveInKeyWindow|NSTrackingInVisibleRect owner:self userInfo:nil];[self addTrackingArea:self.tracking];}
- (void)mouseEntered:(NSEvent*)event{self.hovered=YES;self.needsDisplay=YES;}
- (void)mouseExited:(NSEvent*)event{self.hovered=NO;self.needsDisplay=YES;}
- (void)drawRect:(NSRect)dirty{unsigned state=self.state==NSControlStateValueOn?3:self.cell.highlighted?2:self.hovered&&self.enabled?1:0;[self.sheet drawInRect:self.bounds fromRect:NSMakeRect(0,self.sheet.size.height-(state+1)*26,108,26) operation:NSCompositingOperationSourceOver fraction:self.enabled?1:.5 respectFlipped:YES hints:nil];NSMutableParagraphStyle* p=[NSMutableParagraphStyle new];p.alignment=NSTextAlignmentCenter;[self.title drawInRect:NSMakeRect(0,5,108,20) withAttributes:@{NSFontAttributeName:mu::ui::source_font(12),NSForegroundColorAttributeName:self.enabled?NSColor.whiteColor:NSColor.grayColor,NSParagraphStyleAttributeName:p}];}
@end
@interface MuCreateCanvas:NSView
@property(nonatomic) unsigned selectedClass;
@property(nonatomic,strong) NSImage* input;
@property(nonatomic,strong) NSArray<NSString*>* statNames;
@property(nonatomic,strong) NSArray<NSString*>* descriptions;
@end
@implementation MuCreateCanvas
- (BOOL)isFlipped{return YES;}
- (NSView*)hitTest:(NSPoint)p{return nil;}
- (void)drawRect:(NSRect)dirty{[self.input drawInRect:NSMakeRect(0,317,346,38) fromRect:NSMakeRect(0,self.input.size.height-38,346,38) operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];[[NSColor colorWithWhite:0 alpha:143./255] setFill];NSRectFill(NSMakeRect(346,24,108,self.selectedClass==4?96:80));NSRectFill(NSMakeRect(0,355,454,51));constexpr unsigned stats[7][5]={{18,18,15,30,0},{28,20,25,10,0},{22,25,20,15,0},{26,26,26,26,0},{26,20,20,15,25},{21,21,18,23,0},{32,27,25,20,0}};for(unsigned i=0;i<(self.selectedClass==4?5:4);i++){[self.statNames[i] drawAtPoint:NSMakePoint(368,34+i*17) withAttributes:@{NSFontAttributeName:mu::ui::source_font(12),NSForegroundColorAttributeName:NSColor.whiteColor}];[[NSString stringWithFormat:@"%u",stats[self.selectedClass][i]] drawAtPoint:NSMakePoint(422,34+i*17) withAttributes:@{NSFontAttributeName:mu::ui::source_font(12),NSForegroundColorAttributeName:[NSColor colorWithRed:1 green:.6 blue:.2 alpha:1]}];}[self.descriptions[self.selectedClass] drawInRect:NSMakeRect(10,367,434,38) withAttributes:@{NSFontAttributeName:mu::ui::source_font(12),NSForegroundColorAttributeName:NSColor.whiteColor}];}
@end
@implementation MuCharacterCreatePanel {NSView* _content;MuCreateCanvas* _canvas;MuCreationPreview* _preview;NSArray<MuClassButton*>* _classes;NSButton* _ok;NSButton* _cancel;NSTextField* _nameField;unsigned _selectedClass,_mask;BOOL _busy;}
- (BOOL)isFlipped{return YES;}
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language previews:(std::shared_ptr<const mu::lobby::CreationPreviews>)previews{if((self=[super initWithFrame:NSMakeRect(0,0,800,600)])){auto texts=mu::ui::load_texts(std::filesystem::path(root)/"Data",language);auto value=[&](unsigned id){return [NSString stringWithUTF8String:texts[id].c_str()]?:@"";};_content=[[NSView alloc] initWithFrame:NSMakeRect(173,97,454,406)];[self addSubview:_content];_preview=[[MuCreationPreview alloc] initWithPreviews:previews];_preview.frame=NSMakeRect(0,71,410,335);[_content addSubview:_preview];_canvas=[[MuCreateCanvas alloc] initWithFrame:NSMakeRect(0,0,454,406)];_canvas.input=mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface","cha_id.tga");_canvas.statNames=@[value(1701),value(1702),value(1703),value(1704),value(1738)];_canvas.descriptions=@[value(1705),value(1706),value(1707),value(1708),value(1709),value(1690),value(3152)];[_content addSubview:_canvas];NSMutableArray* classes=[NSMutableArray array];constexpr unsigned textIDs[7]={20,21,22,23,24,1687,3150};constexpr unsigned y[7]={131,157,183,272,298,209,246};for(unsigned c=0;c<7;c++){auto b=[[MuClassButton alloc] initWithFrame:NSMakeRect(346,406-y[c]-26,108,26)];b.sheet=mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface","cha_bt.tga");b.title=value(textIDs[c]);[b setAccessibilityLabel:b.title];b.tag=c;b.target=self;b.action=@selector(choose:);b.bordered=NO;[_content addSubview:b];[classes addObject:b];}_classes=classes;_nameField=[[NSTextField alloc] initWithFrame:NSMakeRect(78,406-338-16,240,16)];_nameField.font=mu::ui::source_font(12);_nameField.textColor=NSColor.whiteColor;_nameField.backgroundColor=NSColor.clearColor;_nameField.drawsBackground=NO;_nameField.bordered=NO;_nameField.focusRingType=NSFocusRingTypeNone;_nameField.target=self;_nameField.action=@selector(confirm:);[_nameField setAccessibilityLabel:value(450)];[_content addSubview:_nameField];_ok=mu_lobby_sprite_button(root,"message_ok_b_all.tga",value(228),NSMakeRect(346,51,54,30),self,@selector(confirm:));_cancel=mu_lobby_sprite_button(root,"loding_cancel_b_all.tga",value(229),NSMakeRect(400,51,54,30),self,@selector(cancel:));[_content addSubview:_ok];[_content addSubview:_cancel];_selectedClass=1;[self configureClassMask:0];self.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;self.hidden=YES;}return self;}
- (void)layout{[super layout];_content.frame=NSMakeRect((self.bounds.size.width-454)/2,(self.bounds.size.height-406)/2,454,406);}
- (NSTextField*)nameField{return _nameField;}
- (unsigned)selectedClass{return _selectedClass;}
- (void)configureClassMask:(unsigned)mask{_mask=mask;constexpr unsigned bit[7]={0,0,0,4,2,1,8};for(unsigned i=0;i<7;i++){_classes[i].enabled=!_busy&&(!bit[i]||(_mask&bit[i]));_classes[i].state=i==_selectedClass?NSControlStateValueOn:NSControlStateValueOff;_classes[i].needsDisplay=YES;}_canvas.selectedClass=_selectedClass;_canvas.needsDisplay=YES;}
- (void)setBusy:(BOOL)busy{_busy=busy;_nameField.enabled=!busy;_ok.enabled=!busy;_cancel.enabled=!busy;[self configureClassMask:_mask];}
- (void)choose:(NSButton*)button{_selectedClass=unsigned(button.tag);[_preview choose:_selectedClass];[self configureClassMask:_mask];}
- (void)confirm:(id)sender{if(!_busy&&self.onCreate)self.onCreate();}
- (void)cancel:(id)sender{if(!_busy&&self.onCancel)self.onCancel();}
- (void)cancelOperation:(id)sender{[self cancel:sender];}
- (void)setHidden:(BOOL)hidden{[super setHidden:hidden];_preview.playing=!hidden;if(!hidden){_nameField.stringValue=@"";[self.window makeFirstResponder:_nameField];}}
@end
@interface MuCharacterMessageBox:NSView
@property(nonatomic,strong) NSImage* background;
@property(nonatomic,strong) NSImage* input;
@property(nonatomic) BOOL inputVisible;
@end
@implementation MuCharacterMessageBox
- (BOOL)isFlipped{return YES;}
- (void)drawRect:(NSRect)dirty{[self.background drawInRect:self.bounds fromRect:NSMakeRect(0,self.background.size.height-113,352,113) operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];if(self.inputVisible)[self.input drawInRect:NSMakeRect(32,76,171,23) fromRect:NSMakeRect(0,self.input.size.height-23,171,23) operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];}
@end
@implementation MuCharacterMessage {MuCharacterMessageBox* _box;NSTextField* _label;NSSecureTextField* _codeField;NSButton* _ok;NSButton* _cancel;}
- (BOOL)isFlipped{return YES;}
- (instancetype)initWithRoot:(const std::string&)root{if((self=[super initWithFrame:NSMakeRect(0,0,800,600)])){_box=[[MuCharacterMessageBox alloc] initWithFrame:NSMakeRect(224,243,352,113)];auto folder=std::filesystem::path(root)/"Data/Interface";_box.background=mu::viewer::load_interface_image(folder,"message_back.tga");_box.input=mu::viewer::load_interface_image(folder,"delete_secret_number.tga");[self addSubview:_box];_label=[NSTextField wrappingLabelWithString:@""];_label.frame=NSMakeRect(20,18,312,48);_label.font=mu::ui::source_font(12);_label.textColor=NSColor.whiteColor;_label.alignment=NSTextAlignmentCenter;[_box addSubview:_label];_codeField=[[NSSecureTextField alloc] initWithFrame:NSMakeRect(42,80,151,16)];_codeField.font=mu::ui::source_font(12);_codeField.bordered=NO;_codeField.drawsBackground=NO;_codeField.textColor=NSColor.whiteColor;_codeField.focusRingType=NSFocusRingTypeNone;[_box addSubview:_codeField];_ok=mu_lobby_sprite_button(root,"message_ok_b_all.tga",@"OK",NSMakeRect(98,72,54,30),self,@selector(confirm:));_cancel=mu_lobby_sprite_button(root,"loding_cancel_b_all.tga",@"Cancel",NSMakeRect(200,72,54,30),self,@selector(cancel:));[_box addSubview:_ok];[_box addSubview:_cancel];self.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;self.hidden=YES;}return self;}
- (void)layout{[super layout];_box.frame=NSMakeRect((self.bounds.size.width-352)/2,(self.bounds.size.height-113)/2,352,113);}
- (NSSecureTextField*)codeField{return _codeField;}
- (void)showText:(NSString*)text input:(BOOL)input confirm:(BOOL)confirm{_label.stringValue=text;_box.inputVisible=input;_box.needsDisplay=YES;_codeField.hidden=!input;_codeField.stringValue=@"";_codeField.enabled=YES;_ok.enabled=YES;_cancel.enabled=YES;_ok.hidden=NO;_cancel.hidden=!confirm;_ok.frame=NSMakeRect(input?209:confirm?98:149,72,54,30);_cancel.frame=NSMakeRect(input?264:200,72,54,30);self.hidden=NO;if(input)[self.window makeFirstResponder:_codeField];}
- (void)setWaiting:(BOOL)waiting{_ok.enabled=!waiting;_cancel.enabled=!waiting;_ok.hidden=waiting;_cancel.hidden=waiting;_codeField.enabled=!waiting;}
- (void)confirm:(id)sender{if(self.onOK)self.onOK();}
- (void)cancel:(id)sender{_codeField.stringValue=@"";if(self.onCancel)self.onCancel();}
@end
