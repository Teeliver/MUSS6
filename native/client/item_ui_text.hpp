#pragma once
#include "item_catalog.hpp"
#include "inventory.hpp"
#include "localization.hpp"
#import <Cocoa/Cocoa.h>
namespace mu::ui { NSColor* item_background_color(const mu::client::Item&,const ItemAttribute&);NSString* item_name(const mu::client::Item&,const ItemCatalog&);NSAttributedString* item_tooltip_styled(const mu::client::Item&,const ItemCatalog&,const Texts&,CGFloat scale,const mu::client::EquipmentAttributes* attributes=nullptr);NSString* item_tooltip(const mu::client::Item&,const ItemCatalog&,const Texts&); }
