#include "sound_bank.hpp"
#include "lobby_view.hpp"
#include "asset_images.hpp"
#include "source_font.hpp"
#include <array>
namespace { const char* images[]={"LSBg01.JPG","LSBg02.JPG","LSBg03.JPG","LSBg04.JPG","login_back.tga","login_me.tga","MU_TITLE.tga","lo_back_s5_im01.jpg","lo_back_s5_im02.jpg","lo_back_s5_im03.jpg","lo_back_s5_im04.jpg","lo_back_s5_im05.jpg","lo_back_s5_im06.jpg","New_lo_back_01.jpg","New_lo_back_02.jpg","lo_back_s5_03.jpg","lo_back_s5_04.jpg","lo_lo.jpg"}; }
void mu::ui::prepare_lobby_images(const std::string& root){
 for(auto file:images)mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface",file);
 for(auto file:{"cha_bt.tga","server_b2_all.tga","server_b2_loding.jpg","server_deco_all.tga","message_ok_b_all.tga","loding_cancel_b_all.tga"})mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface",file);
 for(auto file:{"New_Login_Back01.jpg","New_Login_Back02.jpg","MU-logo.tga"})mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Logo",file);
}
@interface MuLobbyContainer:NSView
@end
@implementation MuLobbyContainer
- (BOOL)isFlipped{return YES;}
- (NSView*)hitTest:(NSPoint)point{NSView* hit=[super hitTest:point];return hit==self?nil:hit;}
@end
@interface MuLoginPanel:MuLobbyContainer
@property(nonatomic,strong) NSArray<NSImage*>* images;
@end
@implementation MuLoginPanel
- (void)drawRect:(NSRect)dirty{
 [self.images[4] drawInRect:NSMakeRect(0,0,329,245) fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];
 for(unsigned i=0;i<2;i++)[self.images[5] drawInRect:NSMakeRect(109,106+i*25,156,23) fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];
}
@end
@interface MuCharacterBalloon:MuLobbyContainer
@property(nonatomic,strong) NSImage* background;
@property(nonatomic,strong) NSArray<NSString*>* lines;
@property(nonatomic,strong) NSColor* nameColor;
@end
@implementation MuCharacterBalloon
- (NSView*)hitTest:(NSPoint)point{return nil;}
- (void)drawRect:(NSRect)dirty{[self.background drawInRect:self.bounds fromRect:NSMakeRect(0,self.background.size.height-54,118,54) operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];NSMutableParagraphStyle* align=[NSMutableParagraphStyle new];align.alignment=NSTextAlignmentCenter;for(unsigned i=0;i<self.lines.count;i++){NSColor* color=i==0?self.nameColor:i==2?[NSColor colorWithRed:1 green:.8 blue:.5 alpha:1]:NSColor.whiteColor;[self.lines[i] drawInRect:NSMakeRect(-10,7+i*16,138,16) withAttributes:@{NSFontAttributeName:mu::ui::source_font(11),NSForegroundColorAttributeName:color,NSParagraphStyleAttributeName:align}];}}
@end
@interface MuLobbyCanvas:MuLobbyContainer
@property(nonatomic,strong) NSArray<NSImage*>* images;
@property(nonatomic) mu::ui::LobbyStage stage;
@property(nonatomic) double progress;
@property(nonatomic) BOOL sceneReady;
@end
@implementation MuLobbyCanvas
- (BOOL)isFlipped{return YES;}
- (void)image:(unsigned)index rect:(NSRect)rect source:(NSRect)source{
 if(!NSEqualRects(source,NSZeroRect))source.origin.y=self.images[index].size.height-source.origin.y-source.size.height;
 [self.images[index] drawInRect:rect fromRect:source operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:!(index<4||(index>=7&&index<20)) hints:nil];
}
- (void)drawRect:(NSRect)dirty{
 if(!self.sceneReady||self.stage==mu::ui::LobbyStage::Boot||self.stage==mu::ui::LobbyStage::Loading){[[NSColor blackColor] setFill];NSRectFill(self.bounds);}
 using S=mu::ui::LobbyStage;
 if(self.stage==S::Loading){
  // LoadingScene.cpp: original 800x600 canvas, 400x512 upper halves,
  // 400x88 lower halves. Texture padding is not stretched into the picture.
  [self image:0 rect:NSMakeRect(0,0,400,512) source:NSMakeRect(0,0,400,512)];
  [self image:1 rect:NSMakeRect(400,0,400,512) source:NSMakeRect(0,0,400,512)];
  [self image:2 rect:NSMakeRect(0,512,400,88) source:NSMakeRect(0,0,400,88)];
  [self image:3 rect:NSMakeRect(400,512,400,88) source:NSMakeRect(0,0,400,88)];
 }else if(self.stage==S::Boot){
  // UIMng.cpp CreateTitleSceneUI: six images in a 1280x1024 layout.
  // Right column and lower row have smaller active regions than the first tile.
  for(unsigned i=0;i<6;i++){CGFloat x=(i%3)*512,y=(i/3)*512+119,w=i%3==2?256:512,h=i<3?512:223;
   [self image:7+i rect:NSMakeRect(x*800/1280,y*600/1024,w*800/1280,h*600/1024) source:NSMakeRect(0,0,w,h)];}
  for(unsigned i=0;i<2;i++){
   [self image:13+i rect:NSMakeRect(i*400,0,400,69) source:NSMakeRect(0,0,400,69)];
   [self image:15+i rect:NSMakeRect(i*400,500,400,100) source:NSMakeRect(0,0,400,100)];
  }
  [self image:6 rect:NSMakeRect(332.5,419.5,135,86.25) source:NSZeroRect];
  CGFloat filled=656*self.progress;for(CGFloat x=0;x<filled;x+=4)[self image:17 rect:NSMakeRect(72+x,540,MIN(4.,filled-x),15) source:NSMakeRect(0,0,MIN(4.,filled-x),15)];
 }else if(!self.sceneReady){
  // Only the original sky backdrop is ready here. The 3D login world must
  // be rendered separately, never substituted with the loading illustration.
  [self image:18 rect:NSMakeRect(0,31.25,400,400) source:NSZeroRect];
  [self image:19 rect:NSMakeRect(400,31.25,400,400) source:NSZeroRect];
 }
 if(self.sceneReady&&(self.stage==S::Servers||self.stage==S::Connecting||self.stage==S::Login||self.stage==S::Authenticating))[self image:20 rect:NSMakeRect(272,31.25,256,128) source:NSZeroRect];

}
@end
@implementation MuLobbyView {
 MuLobbyScene* _scene;MuLobbyScene* _characterScene;MuLobbyCanvas* _canvas;NSView* _serverControls;NSView* _loginControls;NSView* _characterControls;NSTextField* _status;NSMutableArray<MuCharacterBalloon*>* _balloons;std::string _root;mu::ui::Texts _texts;
}
- (void)drawRect:(NSRect)dirty{[NSColor.blackColor setFill];NSRectFill(dirty);}
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language{
 if((self=[super initWithFrame:NSMakeRect(0,0,1100,825)])){
  mu::audio::prepare(std::filesystem::path(root)/"Data");_root=root;_balloons=[NSMutableArray array];_texts=mu::ui::load_texts(std::filesystem::path(root)/"Data",language);
  _canvas=[[MuLobbyCanvas alloc] initWithFrame:self.bounds];[_canvas setBoundsSize:NSMakeSize(800,600)];
  NSMutableArray* loaded=[NSMutableArray array];for(auto file:images)[loaded addObject:mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface",file)];for(auto file:{"New_Login_Back01.jpg","New_Login_Back02.jpg","MU-logo.tga"})[loaded addObject:mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Logo",file)];_canvas.images=loaded;[self addSubview:_canvas];
  _serverControls=[[MuLobbyContainer alloc] initWithFrame:NSMakeRect(0,0,800,600)];_loginControls=[[MuLoginPanel alloc] initWithFrame:NSMakeRect(0,0,329,245)];((MuLoginPanel*)_loginControls).images=loaded;_characterControls=[[MuLobbyContainer alloc] initWithFrame:_serverControls.frame];
  // Child containers use the same top-left coordinate system as source UI.
  [self addSubview:_characterControls];[self addSubview:_serverControls];[self addSubview:_loginControls];
  _status=[NSTextField wrappingLabelWithString:@""];_status.frame=NSMakeRect(60,541,680,45);_status.font=mu::ui::source_font(12);_status.textColor=[NSColor colorWithRed:1 green:.9 blue:.75 alpha:1];_status.alignment=NSTextAlignmentCenter;_status.wantsLayer=YES;_status.layer.backgroundColor=[[NSColor colorWithWhite:0 alpha:.65] CGColor];[_canvas addSubview:_status];
  self.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;[self setStage:mu::ui::LobbyStage::Boot];
 }return self;
}
- (void)attachScene:(std::shared_ptr<const mu::lobby::Scene>)data{[_scene setActive:NO];[_scene removeFromSuperview];_scene=[[MuLobbyScene alloc] initWithData:std::move(data)];[self addSubview:_scene positioned:NSWindowBelow relativeTo:_canvas];_canvas.sceneReady=YES;[self setNeedsLayout:YES];}
- (void)attachCharacterScene:(std::shared_ptr<const mu::lobby::Scene>)data{[_characterScene removeFromSuperview];_characterScene=[[MuLobbyScene alloc] initWithData:std::move(data)];_characterScene.hidden=YES;__weak MuLobbyView* weak=self;_characterScene.onCharacter=^(unsigned row,BOOL enter){MuLobbyView* owner=weak;if(!owner)return;[owner->_characterScene selectCharacter:row];if(owner.onCharacter)owner.onCharacter(row,enter);};[self addSubview:_characterScene positioned:NSWindowBelow relativeTo:_canvas];[self setNeedsLayout:YES];}
- (void)setCharacters:(const std::vector<mu::lobby::CharacterVisual>&)characters{[_characterScene setCharacters:characters];for(NSView* balloon in _balloons)[balloon removeFromSuperview];[_balloons removeAllObjects];constexpr unsigned classText[7][3]={{20,25,1669},{21,26,1668},{22,27,1670},{23,23,1671},{24,24,1672},{1687,1688,1689},{3150,3150,3151}};for(auto& entry:characters){const auto& c=entry.character;unsigned cls=c.appearance[0]>>5,tier=(c.appearance[0]&28)>=28?2:((c.appearance[0]&16)?1:0),guild=488;switch(c.guildStatus){case 0:guild=1330;break;case 32:guild=1302;break;case 64:guild=1301;break;case 128:guild=1300;break;default:break;}auto balloon=[[MuCharacterBalloon alloc] initWithFrame:NSMakeRect(0,0,118,54)];balloon.background=mu::viewer::load_interface_image(std::filesystem::path(_root)/"Data/Interface","character_ex.tga");balloon.nameColor=(c.control&1)?NSColor.cyanColor:(c.control&18)?[NSColor colorWithRed:1 green:.8 blue:.5 alpha:1]:(c.control&40)?NSColor.redColor:NSColor.whiteColor;balloon.lines=@[[NSString stringWithUTF8String:c.name.c_str()],[NSString stringWithFormat:@"(%@)",[self globalText:guild fallback:@""]],[NSString stringWithFormat:@"%@ %u",[self globalText:classText[cls][tier] fallback:@""],c.level]];[_characterControls addSubview:balloon];[_balloons addObject:balloon];}[self setNeedsLayout:YES];}
- (NSView*)canvas{return _canvas;}
- (NSView*)serverControls{return _serverControls;}
- (NSView*)loginControls{return _loginControls;}
- (NSView*)characterControls{return _characterControls;}
- (NSTextField*)status{return _status;}
- (NSString*)globalText:(unsigned)index fallback:(NSString*)fallback{auto i=_texts.find(index);return i==_texts.end()?fallback:([NSString stringWithUTF8String:i->second.c_str()]?:fallback);}
- (void)setCompleted:(unsigned)completed total:(unsigned)total{_canvas.progress=total?MIN(double(completed)/total,1.):0;_canvas.needsDisplay=YES;}
- (void)setStage:(mu::ui::LobbyStage)stage{auto previous=_canvas.stage;if(stage!=previous){if(stage==mu::ui::LobbyStage::Servers){mu::audio::play(@"iTitle.wav",.35);mu::audio::ambient(@"aWind.wav");}else if(stage==mu::ui::LobbyStage::Connecting)mu::audio::play(@"iButtonClick.wav",.5);else if(stage==mu::ui::LobbyStage::Login)mu::audio::play(@"iCreateWindow.wav",.4);else if(stage==mu::ui::LobbyStage::Characters){mu::audio::ambient(@"aWind.wav");mu::audio::play(@"iFLogInAlert.wav",.4);}else if(stage==mu::ui::LobbyStage::World)mu::audio::ambient(nil);}_canvas.stage=stage;_status.hidden=stage==mu::ui::LobbyStage::Boot||stage==mu::ui::LobbyStage::Servers||stage==mu::ui::LobbyStage::Characters;BOOL sceneVisible=stage==mu::ui::LobbyStage::Servers||stage==mu::ui::LobbyStage::Connecting||stage==mu::ui::LobbyStage::Login||stage==mu::ui::LobbyStage::Authenticating;_scene.hidden=!sceneVisible;[_scene setActive:sceneVisible];BOOL charactersVisible=stage==mu::ui::LobbyStage::Characters;_characterScene.hidden=!charactersVisible;[_characterScene setActive:charactersVisible];_serverControls.hidden=stage!=mu::ui::LobbyStage::Servers&&stage!=mu::ui::LobbyStage::Connecting;_loginControls.hidden=stage!=mu::ui::LobbyStage::Login&&stage!=mu::ui::LobbyStage::Authenticating;_characterControls.hidden=stage!=mu::ui::LobbyStage::Characters;_canvas.needsDisplay=YES;}
- (void)layout{[super layout];CGFloat scale=MIN(self.bounds.size.width/800,self.bounds.size.height/600);_canvas.frame=NSMakeRect((self.bounds.size.width-800*scale)/2,(self.bounds.size.height-600*scale)/2,800*scale,600*scale);[_canvas setBoundsSize:NSMakeSize(800,600)];_serverControls.frame=self.bounds;_characterControls.frame=self.bounds;_loginControls.frame=NSMakeRect((self.bounds.size.width-329)/2,(self.bounds.size.height-245)/2,329,245);_scene.frame=NSInsetRect(_canvas.frame,0,31.25*scale);_characterScene.frame=_scene.frame;for(unsigned i=0;i<_balloons.count;i++){NSPoint anchor=[_characterScene characterAnchor:i];anchor=[_characterControls convertPoint:anchor fromView:_characterScene];_balloons[i].frame=NSMakeRect(anchor.x-59,anchor.y-54,118,54);}}
@end
@interface MuLobbySpriteButton:NSButton
@property(nonatomic,strong) NSImage* sheet;
@property(nonatomic) BOOL hovered;
@property(nonatomic,strong) NSTrackingArea* muTracking;
@end
@implementation MuLobbySpriteButton
- (void)mouseDown:(NSEvent*)event{if(self.enabled)mu::audio::play(@"iButtonClick.wav",.4);[super mouseDown:event];}
- (BOOL)isFlipped{return YES;}
- (void)updateTrackingAreas{[super updateTrackingAreas];if(self.muTracking)[self removeTrackingArea:self.muTracking];self.muTracking=[[NSTrackingArea alloc] initWithRect:NSZeroRect options:NSTrackingMouseEnteredAndExited|NSTrackingActiveInKeyWindow|NSTrackingInVisibleRect owner:self userInfo:nil];[self addTrackingArea:self.muTracking];}
- (void)mouseEntered:(NSEvent*)event{self.hovered=YES;self.needsDisplay=YES;}
- (void)mouseExited:(NSEvent*)event{self.hovered=NO;self.needsDisplay=YES;}
- (void)drawRect:(NSRect)dirty{unsigned state=!self.enabled&&self.sheet.size.height>=120?3:(self.cell.highlighted?2:(self.hovered&&self.enabled?1:0));NSRect source=NSMakeRect(0,self.sheet.size.height-(state+1)*30,54,30);[self.sheet drawInRect:self.bounds fromRect:source operation:NSCompositingOperationSourceOver fraction:self.enabled||self.sheet.size.height>=120?1:.4 respectFlipped:YES hints:nil];}
@end
NSButton* mu_lobby_sprite_button(const std::string& root,const char* filename,NSString* title,NSRect frame,id target,SEL action){
 auto b=[[MuLobbySpriteButton alloc] initWithFrame:frame];b.sheet=mu::viewer::load_interface_image(std::filesystem::path(root)/"Data/Interface",filename);b.title=title;[b setAccessibilityLabel:title];b.target=target;b.action=action;b.bordered=NO;return b;
}
