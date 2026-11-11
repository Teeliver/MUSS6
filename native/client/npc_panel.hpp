#include "item_drag.hpp"
#pragma once
#include "client.hpp"
#import <Cocoa/Cocoa.h>
@interface MuWarehousePanel:NSObject
@property(nonatomic,copy) void (^onClose)(void);
@property(nonatomic,copy) void (^onBuy)(unsigned slot);
@property(nonatomic) BOOL buying;
@property(nonatomic,copy) void (^onRepairMode)(BOOL enabled);
@property(nonatomic,copy) void (^onRepairAll)(void);
@property(nonatomic,strong) NSView* view;
@property(nonatomic,strong) MuItemDrag* dragController;
- (instancetype)initWithState:(const mu::client::NpcState&)state assetRoot:(const std::string&)root language:(const std::string&)language;
- (void)attachToGameView:(NSView*)game;
- (void)attachToGameView:(NSView*)game rightOffset:(unsigned)offset;
- (void)updateState:(const mu::client::NpcState&)state;
- (void)close;
@end
