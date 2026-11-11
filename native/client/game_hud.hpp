#pragma once
#include "client.hpp"
#import <Cocoa/Cocoa.h>
#import <SceneKit/SceneKit.h>
@interface MuGameHud:NSView
@property(nonatomic,copy) void (^onInventory)(void);
@property(nonatomic,copy) void (^onCharacter)(void);
@property(nonatomic,copy) void (^onMenu)(void);
@property(nonatomic,readonly) unsigned activeSkill;
@property(nonatomic) BOOL groundNamesLocked;
- (void)assignSkillHotkey:(unsigned)index;
- (void)selectSkillHotkey:(unsigned)index;
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language scene:(SCNView*)scene actor:(SCNNode*)actor name:(const std::string&)name;
- (void)updateState:(const mu::client::WorldState&)state;
- (void)addNotice:(const mu::client::ServerNotice&)notice;
- (void)stop;
@end
