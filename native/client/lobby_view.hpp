#pragma once
#import <Cocoa/Cocoa.h>
#include <string>
#include "localization.hpp"
#include "lobby_scene.hpp"
// Workflow states are distinct from renderer scenes. Only the controller changes them
// after a protocol result; a loading illustration never implies server success.
namespace mu::ui {
enum class LobbyStage { Boot, Servers, Connecting, Login, Authenticating, Characters, Loading, World };
void prepare_lobby_images(const std::string& root);
}
@interface MuLobbyView : NSView
@property(nonatomic,strong,readonly) NSView* canvas;
@property(nonatomic,strong,readonly) NSView* serverControls;
@property(nonatomic,strong,readonly) NSView* loginControls;
@property(nonatomic,strong,readonly) NSView* characterControls;
@property(nonatomic,strong,readonly) NSTextField* status;
- (instancetype)initWithRoot:(const std::string&)root language:(const std::string&)language;
- (void)attachScene:(std::shared_ptr<const mu::lobby::Scene>)scene;
- (void)attachCharacterScene:(std::shared_ptr<const mu::lobby::Scene>)scene;
- (void)setCharacters:(const std::vector<mu::lobby::CharacterVisual>&)characters;
@property(nonatomic,copy) void (^onCharacter)(unsigned row,BOOL enter);
- (void)setCompleted:(unsigned)completed total:(unsigned)total;
- (void)setStage:(mu::ui::LobbyStage)stage;
- (NSString*)globalText:(unsigned)index fallback:(NSString*)fallback;
@end
// Original vertical button sheets: normal=0, pressed=1, hovered=2.
NSButton* mu_lobby_sprite_button(const std::string& root,const char* filename,NSString* accessibilityTitle,NSRect frame,id target,SEL action);
