#pragma once
#include "client.hpp"
#import <Cocoa/Cocoa.h>
@interface MuItemDrag:NSObject
@property(nonatomic) BOOL pending;
@property(nonatomic) unsigned baseClass;
@property(nonatomic,readonly) BOOL carrying;
- (void)setAttributes:(const mu::client::EquipmentAttributes&)attributes;
- (mu::client::EquipmentAttributes)attributes;
- (BOOL)hidesItemInContainer:(unsigned)container slot:(unsigned)slot;
@property(nonatomic,weak) NSView* sellView;
@property(nonatomic,weak) NSView* dropView;
@property(nonatomic,copy) void (^onDrop)(unsigned slot);
@property(nonatomic,copy) void (^onSell)(unsigned slot);
@property(nonatomic,copy) void (^onTransfer)(mu::client::ItemMove);
- (void)registerContainer:(unsigned)container view:(NSView*)view items:(const mu::client::ItemSlots&)items root:(const std::string&)root language:(const std::string&)language;
- (void)begin:(NSEvent*)event inView:(NSView*)view;
- (void)drag:(NSEvent*)event;
- (BOOL)finish:(NSEvent*)event;
- (void)cancel;
@end
