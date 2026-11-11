#pragma once
#import <Cocoa/Cocoa.h>
@interface MuItemTooltip:NSView
- (void)showText:(NSString*)text atPoint:(NSPoint)point inView:(NSView*)host scale:(CGFloat)scale;
- (void)showStyledText:(NSAttributedString*)text atPoint:(NSPoint)point inView:(NSView*)host scale:(CGFloat)scale;
- (void)hide;
@end
