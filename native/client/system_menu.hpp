#pragma once
#include "lobby_view.hpp"
#ifdef __OBJC__
@interface MuSystemMenu:NSView
- (instancetype)initWithRoot:(const std::string&)root lobby:(MuLobbyView*)lobby;
- (void)presentIn:(NSView*)view world:(BOOL)world authenticated:(BOOL)authenticated;
- (void)close;
@property(nonatomic,copy) void (^onAction)(unsigned action);
@end
#endif
