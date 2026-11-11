#pragma once
#include "client.hpp"
#import <Cocoa/Cocoa.h>
// Fixed in-game character information; never changes the world camera.
@interface MuCharacterInfo:NSView
@property(nonatomic,copy) void (^onClose)(void);
@property(nonatomic,copy) void (^onAddStat)(unsigned type);
@property(nonatomic) BOOL pending;
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language name:(const std::string&)name;
- (void)updateState:(const mu::client::WorldState&)state;
- (void)attach:(NSView*)game;
- (void)close;
@end
