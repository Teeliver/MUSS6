#pragma once
#include "inventory.hpp"
#include <string>
#import <Cocoa/Cocoa.h>
namespace mu::ui {
void prepare_item_icons(const std::string& root,const std::string& language,const mu::client::ItemSlots& items);
NSImage* cached_item_icon(const std::string& root,const std::string& language,const mu::client::Item& item);
}
#import <SceneKit/SceneKit.h>
namespace mu::ui {
SCNScene* cached_item_scene(const std::string& root,const std::string& language,const mu::client::Item& item);
}
@interface MuHoveredItemView:SCNView
- (void)showItem:(const mu::client::Item&)item root:(const std::string&)root language:(const std::string&)language rect:(NSRect)rect inView:(NSView*)host;
- (void)setRotating:(BOOL)rotating;
- (void)hide;
@end
namespace mu::ui {struct ItemPresentation{unsigned slot;mu::client::Item item;NSRect rect;};}
@interface MuItemEffectsLayer:SCNView
- (void)updateItems:(const std::vector<mu::ui::ItemPresentation>&)items root:(const std::string&)root language:(const std::string&)language;
- (BOOL)rendersSlot:(unsigned)slot;
- (void)hideSlot:(NSNumber*)slot;
- (void)stop;
@end
