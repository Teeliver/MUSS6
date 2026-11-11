#include "server_selector.hpp"
#include "asset_images.hpp"
#include "source_font.hpp"
#include <map>
#include <algorithm>
namespace {
void sprite(NSImage* image,NSRect target,NSRect source,CGFloat alpha=1){source.origin.y=image.size.height-source.origin.y-source.size.height;[image drawInRect:target fromRect:source operation:NSCompositingOperationSourceOver fraction:alpha respectFlipped:YES hints:nil];}
}
@interface MuServerButton:NSButton
@property(nonatomic,strong) NSImage* sheet;
@property(nonatomic,strong) NSImage* gauge;
@property(nonatomic) unsigned load;
@property(nonatomic) CGFloat spriteWidth;
@property(nonatomic) BOOL group,selected,hovered;
@property(nonatomic,strong) NSTrackingArea* muTracking;
@end
@implementation MuServerButton
- (BOOL)isFlipped{return YES;}
- (void)updateTrackingAreas{[super updateTrackingAreas];if(self.muTracking)[self removeTrackingArea:self.muTracking];self.muTracking=[[NSTrackingArea alloc] initWithRect:NSZeroRect options:NSTrackingMouseEnteredAndExited|NSTrackingActiveInKeyWindow|NSTrackingInVisibleRect owner:self userInfo:nil];[self addTrackingArea:self.muTracking];}
- (void)mouseEntered:(NSEvent*)event{self.hovered=YES;self.needsDisplay=YES;}
- (void)mouseExited:(NSEvent*)event{self.hovered=NO;self.needsDisplay=YES;}
- (void)drawRect:(NSRect)dirty{
 unsigned state=self.cell.highlighted?1:(self.hovered?2:(self.group&&self.selected?3:0));
 sprite(self.sheet,self.bounds,NSMakeRect(0,state*26,self.spriteWidth,26),self.enabled?1:.45);
 if(!self.group&&self.load<128){CGFloat fill=160*MIN(self.load,100u)/100.;if(fill>0)sprite(self.gauge,NSMakeRect(16,19,fill,4),NSMakeRect(0,0,fill,4));}
 auto paragraph=[NSMutableParagraphStyle new];paragraph.alignment=NSTextAlignmentCenter;paragraph.lineBreakMode=NSLineBreakByTruncatingTail;
 [self.title drawInRect:NSMakeRect(3,5,self.bounds.size.width-6,16) withAttributes:@{NSFontAttributeName:mu::ui::source_font(10),NSForegroundColorAttributeName:self.enabled?NSColor.whiteColor:NSColor.grayColor,NSParagraphStyleAttributeName:paragraph}];
}
@end
@implementation MuServerSelector {
 std::vector<mu::client::Server> _servers;mu::ui::Texts _texts;std::map<unsigned,std::vector<unsigned>> _groups;unsigned _selectedGroup;BOOL _enabled;
 NSImage* _groupSheet;NSImage* _serverSheet;NSImage* _gauge;NSImage* _deco;
}
- (BOOL)isFlipped{return YES;}
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language{
 if((self=[super initWithFrame:NSMakeRect(0,0,800,510)])){
  auto path=std::filesystem::path(root)/"Data/Interface";_texts=mu::ui::load_texts(std::filesystem::path(root)/"Data",language);
  _groupSheet=mu::viewer::load_interface_image(path,"cha_bt.tga");_serverSheet=mu::viewer::load_interface_image(path,"server_b2_all.tga");_gauge=mu::viewer::load_interface_image(path,"server_b2_loding.jpg");_deco=mu::viewer::load_interface_image(path,"server_deco_all.tga");_enabled=YES;_selectedGroup=~0u;
 }return self;
}
- (NSString*)text:(unsigned)index fallback:(NSString*)fallback{auto i=_texts.find(index);return i==_texts.end()?fallback:([NSString stringWithUTF8String:i->second.c_str()]?:fallback);}
- (void)setServers:(const std::vector<mu::client::Server>&)servers{
 _servers=servers;_groups.clear();for(unsigned i=0;i<servers.size();i++)_groups[servers[i].code/20].push_back(i);
 if(!_groups.count(_selectedGroup))_selectedGroup=~0u;[self rebuild];
}
- (void)setFrameSize:(NSSize)size{[super setFrameSize:size];if(_groupSheet)[self rebuild];}
- (void)resetSelection{_selectedGroup=~0u;[self rebuild];}
- (BOOL)enabled{return _enabled;}
- (void)setEnabled:(BOOL)value{_enabled=value;for(NSView* view in self.subviews)if([view isKindOfClass:MuServerButton.class]){auto b=(MuServerButton*)view;b.enabled=value&&(b.group||b.load<100);b.needsDisplay=YES;}}
- (void)rebuild{
 for(NSView* view in [self.subviews copy])[view removeFromSuperview];
 auto selected=_groups.find(_selectedGroup);unsigned subRows=selected==_groups.end()?0:selected->second.size();unsigned groupRows=std::min<std::size_t>(10,_groups.size());
 CGFloat left=(self.bounds.size.width-465)/2,top=MAX(30.,self.bounds.size.height/2-96);
 unsigned index=0;for(auto& group:_groups){auto b=[[MuServerButton alloc] initWithFrame:NSMakeRect(left+(index<10?0:357),top+(index%10)*26,108,26)];b.group=YES;b.sheet=_groupSheet;b.spriteWidth=108;b.selected=group.first==_selectedGroup;b.title=[self text:540+group.first fallback:[NSString stringWithFormat:@"Server %u",group.first]];b.tag=group.first;b.target=self;b.action=@selector(selectGroup:);b.bordered=NO;b.enabled=_enabled;[b setAccessibilityLabel:b.title];[self addSubview:b];index++;}
 auto found=_groups.find(_selectedGroup);if(found!=_groups.end()){unsigned n=0;for(auto row:found->second){const auto& server=_servers[row];auto b=[[MuServerButton alloc] initWithFrame:NSMakeRect(left+136,top+n++*26,193,26)];b.sheet=_serverSheet;b.spriteWidth=193;b.gauge=_gauge;b.load=server.load;
  NSString* group=[self text:540+_selectedGroup fallback:@"Server"];NSString* status=[self text:server.load>=128?560:(server.load>=100?561:562) fallback:@""];
  b.title=[NSString stringWithFormat:@"%@-%u %@",group,server.code%20+1,status];b.tag=row;b.target=self;b.action=@selector(choose:);b.bordered=NO;b.enabled=_enabled&&server.load<100;[b setAccessibilityLabel:b.title];[self addSubview:b];}}
 self.needsDisplay=YES;
}
- (void)selectGroup:(MuServerButton*)button{if(!_enabled)return;_selectedGroup=_selectedGroup==unsigned(button.tag)?~0u:unsigned(button.tag);[self rebuild];}
- (void)choose:(MuServerButton*)button{if(_enabled&&button.enabled&&self.onChoose)self.onChoose(unsigned(button.tag));}
- (void)drawRect:(NSRect)dirty{for(NSView* view in self.subviews)if([view isKindOfClass:MuServerButton.class]&&((MuServerButton*)view).group){sprite(_deco,NSMakeRect(view.frame.origin.x-8,view.frame.origin.y-19,68,95),NSMakeRect(0,0,68,95));break;}}
@end
