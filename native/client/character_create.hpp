#pragma once
#include "character_scene.hpp"
#include <array>
namespace mu::lobby {
using CreationPreviews=std::array<std::array<std::shared_ptr<const mu::viewer::ActorVisual>,2>,7>;
std::shared_ptr<const CreationPreviews> load_creation_previews(const std::string& root);
}
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
@interface MuCharacterCreatePanel:NSView
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language previews:(std::shared_ptr<const mu::lobby::CreationPreviews>)previews;
- (void)configureClassMask:(unsigned)mask;
- (void)setBusy:(BOOL)busy;
@property(nonatomic,strong,readonly) NSTextField* nameField;
@property(nonatomic,readonly) unsigned selectedClass;
@property(nonatomic,copy) void (^onCreate)(void);
@property(nonatomic,copy) void (^onCancel)(void);
@end
@interface MuCharacterMessage:NSView
- (instancetype)initWithRoot:(const std::string&)root;
- (void)setWaiting:(BOOL)waiting;
- (void)showText:(NSString*)text input:(BOOL)input confirm:(BOOL)confirm;
@property(nonatomic,strong,readonly) NSSecureTextField* codeField;
@property(nonatomic,copy) void (^onOK)(void);
@property(nonatomic,copy) void (^onCancel)(void);
@end
#endif
