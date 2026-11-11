#pragma once
#import <Cocoa/Cocoa.h>
#include "client.hpp"
#include "localization.hpp"
@interface MuServerSelector:NSView
@property(nonatomic,copy) void (^onChoose)(unsigned row);
@property(nonatomic) BOOL enabled;
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language;
- (void)resetSelection;
- (void)setServers:(const std::vector<mu::client::Server>&)servers;
@end
