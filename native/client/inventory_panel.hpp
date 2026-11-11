#include "item_drag.hpp"
#pragma once
#include "client.hpp"
#import <Cocoa/Cocoa.h>
namespace mu::ui {void preload_item_panel_assets(const std::string& root,const std::string& language="Eng");}
@interface MuInventoryPanel:NSObject
@property(nonatomic,strong) NSView* view;
@property(nonatomic,strong) MuItemDrag* dragController;
@property(nonatomic,copy) void (^onClose)(void);
@property(nonatomic) BOOL repairMode;
@property(nonatomic,copy) void (^onUseBook)(unsigned slot);
@property(nonatomic,copy) void (^onRepair)(unsigned slot);
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language;
- (void)updateState:(const mu::client::WorldState&)state;
- (void)attachToGameView:(NSView*)game;
- (void)close;
@end
