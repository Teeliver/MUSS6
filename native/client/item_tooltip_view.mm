#include "source_font.hpp"
#include "item_tooltip_view.hpp"
#include <algorithm>
#include <cmath>
@implementation MuItemTooltip{NSAttributedString* content;CGFloat contentPadding;}
- (BOOL)isFlipped{return YES;}
- (NSView*)hitTest:(NSPoint)point{return nil;}
- (void)showText:(NSString*)text atPoint:(NSPoint)point inView:(NSView*)host scale:(CGFloat)scale{auto paragraph=[NSMutableParagraphStyle new];paragraph.alignment=NSTextAlignmentCenter;auto styled=[[NSAttributedString alloc] initWithString:text attributes:@{NSFontAttributeName:mu::ui::source_font(9*scale),NSForegroundColorAttributeName:NSColor.whiteColor,NSParagraphStyleAttributeName:paragraph}];[self showStyledText:styled atPoint:point inView:host scale:scale];}
- (void)showStyledText:(NSAttributedString*)styled atPoint:(NSPoint)point inView:(NSView*)host scale:(CGFloat)scale{
 if(!styled.length||!host){[self hide];return;}contentPadding=2*scale;CGFloat padding=contentPadding,maxWidth=std::min(230*scale,host.bounds.size.width-padding*2);auto rect=[styled boundingRectWithSize:NSMakeSize(maxWidth,std::max(1.,host.bounds.size.height-padding*2)) options:NSStringDrawingUsesLineFragmentOrigin];CGFloat width=std::min(host.bounds.size.width,std::ceil(rect.size.width)+padding*2),height=std::min(host.bounds.size.height,std::ceil(rect.size.height)+padding*2);content=styled;self.frame=NSMakeRect(std::clamp(point.x-width/2,0.,host.bounds.size.width-width),std::clamp(point.y-height-12,0.,host.bounds.size.height-height),width,height);if(self.superview!=host){[self removeFromSuperview];[host addSubview:self];}self.needsDisplay=YES;
}
- (void)drawRect:(NSRect)dirty{[[NSColor colorWithWhite:0 alpha:.65] setFill];NSRectFill(self.bounds);[content drawWithRect:NSInsetRect(self.bounds,contentPadding,contentPadding) options:NSStringDrawingUsesLineFragmentOrigin];}
- (void)hide{[self removeFromSuperview];}
@end
