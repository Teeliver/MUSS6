#include "system_menu.hpp"
#include "asset_images.hpp"
#include "source_font.hpp"
@interface MuMenuCanvas:NSView
@property(nonatomic,strong) NSArray<NSImage*>* images;
@property(nonatomic) BOOL world;
@end
@implementation MuMenuCanvas
- (BOOL)isFlipped{return YES;}
- (void)image:(unsigned)i rect:(NSRect)rect{[self.images[i] drawInRect:rect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:1 respectFlipped:YES hints:nil];}
- (void)drawRect:(NSRect)dirty{CGFloat h=self.bounds.size.height;if(self.world){[self image:3 rect:NSMakeRect(0,2,222,h-10)];[self image:0 rect:NSMakeRect(0,0,230,67)];for(unsigned i=0;i<5;i++)[self image:1 rect:NSMakeRect(0,67+i*15,230,15)];[self image:2 rect:NSMakeRect(0,142,230,50)];}else{[[NSColor colorWithPatternImage:self.images[3]] setFill];NSRectFill(NSMakeRect(5,5,203,h-10));[self image:0 rect:NSMakeRect(0,0,213,64)];[self image:1 rect:NSMakeRect(0,h-43,213,43)];[self image:2 rect:NSMakeRect(0,64,5,h-107)];[self image:4 rect:NSMakeRect(208,64,5,h-107)];}}
@end
@interface MuMenuButton:NSButton
@property(nonatomic,strong) NSImage* sheet;
@property(nonatomic) unsigned frames;
@property(nonatomic) BOOL hovered;
@end
@implementation MuMenuButton
- (BOOL)isFlipped{return YES;}
- (void)updateTrackingAreas{for(NSTrackingArea* a in self.trackingAreas)[self removeTrackingArea:a];[self addTrackingArea:[[NSTrackingArea alloc] initWithRect:self.bounds options:NSTrackingMouseEnteredAndExited|NSTrackingActiveInActiveApp|NSTrackingInVisibleRect owner:self userInfo:nil]];[super updateTrackingAreas];}
- (void)mouseEntered:(NSEvent*)event{self.hovered=YES;self.needsDisplay=YES;}
- (void)mouseExited:(NSEvent*)event{self.hovered=NO;self.needsDisplay=YES;}
- (void)drawRect:(NSRect)dirty{unsigned state=self.enabled?(self.cell.highlighted?2:self.hovered?1:0):0;CGFloat h=self.frames==4?30:29;[self.sheet drawInRect:self.bounds fromRect:NSMakeRect(0,self.sheet.size.height-(state+1)*h,108,h) operation:NSCompositingOperationSourceOver fraction:self.enabled?1:.5 respectFlipped:YES hints:nil];NSMutableParagraphStyle* p=[NSMutableParagraphStyle new];p.alignment=NSTextAlignmentCenter;[self.title drawInRect:NSMakeRect(0,8,108,16) withAttributes:@{NSFontAttributeName:mu::ui::source_font(11),NSForegroundColorAttributeName:self.enabled?NSColor.whiteColor:NSColor.grayColor,NSParagraphStyleAttributeName:p}];}
@end
@implementation MuSystemMenu{
 std::string _root;__weak MuLobbyView* _lobby;__weak NSResponder* _previousResponder;MuMenuCanvas* _content;BOOL _world;
}
- (instancetype)initWithRoot:(const std::string&)root lobby:(MuLobbyView*)lobby{self=[super initWithFrame:NSZeroRect];if(self){_root=root;_lobby=lobby;self.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;}return self;}
- (BOOL)isFlipped{return YES;}
- (BOOL)acceptsFirstResponder{return YES;}
- (void)mouseDown:(NSEvent*)event{}
- (void)keyDown:(NSEvent*)event{if(event.keyCode==53)[self close];}
- (void)presentIn:(NSView*)view world:(BOOL)world authenticated:(BOOL)authenticated{
 _previousResponder=view.window.firstResponder;[self removeFromSuperview];[_content removeFromSuperview];_world=world;_content=[MuMenuCanvas new];_content.world=world;
 auto dir=std::filesystem::path(_root)/"Data/Interface";
 NSMutableArray* images=[NSMutableArray array];if(world){for(auto file:{"newui_msgbox_top.tga","newui_msgbox_middle.tga","newui_msgbox_bottom.tga","newui_msgbox_back.jpg"})[images addObject:mu::viewer::load_interface_image(dir,file)];}else{for(auto file:{"op1_back1.tga","op1_back2.tga","op1_back3.jpg","op1_stone.jpg","op1_back4.jpg"})[images addObject:mu::viewer::load_interface_image(dir,file)];}_content.images=images;
 CGFloat h=world?192:(authenticated?187:155);_content.frame=NSMakeRect(0,0,world?230:213,h);[self addSubview:_content];
 const unsigned ids[]={381,382,383,385,world?384u:388u};for(unsigned i=0;i<5;i++){if(!world&&(i==2||(i==1&&!authenticated)))continue;auto b=[MuMenuButton new];b.sheet=mu::viewer::load_interface_image(dir,world?"newui_btn_empty.tga":"op1_b_all.tga");b.frames=world?3:4;b.bordered=NO;b.title=[_lobby globalText:ids[i] fallback:@""];[b setAccessibilityLabel:b.title];b.tag=i;b.target=self;b.action=@selector(activate:);CGFloat y=world?23+i*30:i==0?33:i==1?67:i==3?h-86:h-52;b.frame=NSMakeRect((_content.frame.size.width-108)/2,y,108,world?29:30);b.enabled=!(i==1&&!authenticated)&&i!=3;[_content addSubview:b];}
 self.frame=view.bounds;[view addSubview:self positioned:NSWindowAbove relativeTo:nil];self.needsLayout=YES;[self layoutSubtreeIfNeeded];[self.window makeFirstResponder:self];
}
- (void)layout{[super layout];CGFloat w=_content.frame.size.width,h=_content.frame.size.height;_content.frame=NSMakeRect((self.bounds.size.width-w)/2,_world?MAX(0,(self.bounds.size.height-480)/2+100):(self.bounds.size.height-h)/2,w,h);}
- (void)activate:(NSButton*)sender{unsigned action=unsigned(sender.tag);[self close];if(action<4&&self.onAction)self.onAction(action);}
- (void)close{NSWindow* window=self.window;[self removeFromSuperview];if(_previousResponder)[window makeFirstResponder:_previousResponder];}
@end
