#import <Cocoa/Cocoa.h>
#include "client.hpp"
#include "sound_bank.hpp"
#include "ground_controls.hpp"
#include "world_view.hpp"
#include "pathfinding.hpp"
#include "npc_panel.hpp"
#include "inventory_panel.hpp"
#include "character_info.hpp"
#include "item_icons.hpp"
#include "lobby_view.hpp"
#include "character_create.hpp"
#include "system_menu.hpp"
#include "server_selector.hpp"
#include "item_catalog.hpp"
#include "source_font.hpp"
#include "game_hud.hpp"
#include <deque>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
static NSString* text(const std::string& s){return [NSString stringWithUTF8String:s.c_str()]?:@"Invalid text";}
static std::shared_ptr<const mu::viewer::CharacterBody> changedEquipment(const mu::client::Profile& profile,const mu::client::Character& character,const mu::client::WorldState& state,std::string& key){if(!state.inventory||!state.inventory->ready||character.appearance.empty())return {};auto resources=mu::viewer::character_resources(character,state.inventory.get());auto next=resources.key();if(next==key)return {};auto body=mu::viewer::load_character_body(profile.assetDirectory,resources);key=next;return body;}
@interface ClientController:NSObject<NSApplicationDelegate,NSTableViewDataSource,NSTableViewDelegate>{
 mu::client::Profile profile;mu::client::Character activeCharacter;std::string equipmentKey;
 std::vector<mu::client::Server> servers;
 mu::client::CharacterList characterList;
 std::shared_ptr<mu::client::Session> session;
 mu::input::GroundNames groundNames;dispatch_queue_t networkQueue;id keyMonitor;
 BOOL npcWaiting,npcOpen;double npcStarted;unsigned npcIndex;
 unsigned pendingCombatTarget,combatAttempts;BOOL hasCombatTarget,pendingCombatSkill;
 unsigned pendingLoot;BOOL hasPendingLoot;
 BOOL authenticated,working,inWorld,polling,moving,gameConnected,casting;std::uint64_t lastCastRevision,lastDamageRevision,lastLevelRevision;double lastCastSent;double lastInputTime,lastActualInputTime;
 mu::client::WorldState latestState;
 std::deque<unsigned> route;BOOL routeRequested;unsigned destinationX,destinationY;
 std::uint64_t iconInventoryRevision;std::shared_ptr<const mu::client::ItemSlots> iconNpcItems;
 std::shared_ptr<mu::viewer::SceneData> onlineScene;
 std::shared_ptr<const mu::lobby::Scene> lobbyScene,characterScene;unsigned characterVisualRevision;NSInteger selectedCharacterRow;std::shared_ptr<const mu::lobby::CreationPreviews> creationPreviews;std::string pendingDeleteName;
}
@property(nonatomic,strong) NSWindow* window;
@property(nonatomic,strong) MuSystemMenu* systemMenu;
- (void)toggleSystemMenu:(id)sender;
- (void)performSystemAction:(unsigned)action;
@property(nonatomic,strong) MuLobbyView* lobby;
@property(nonatomic,strong) MuServerSelector* serverSelector;
@property(nonatomic,strong) NSButton* connectButton;
@property(nonatomic,strong) NSButton* backButton;
@property(nonatomic,strong) NSButton* cancelLoginButton;
@property(nonatomic,strong) MuWorldViewer* worldViewer;
@property(nonatomic,strong) MuWarehousePanel* warehousePanel;
@property(nonatomic,strong) MuInventoryPanel* inventoryPanel;
@property(nonatomic,strong) MuItemDrag* itemDrag;
@property(nonatomic) BOOL logoutAfterTransfer;
@property(nonatomic,strong) MuGameHud* gameHud;
@property(nonatomic,strong) MuCharacterInfo* characterInfo;
- (void)transferItem:(mu::client::ItemMove)move;
@property(nonatomic,strong) NSTextField *host,*port,*status,*account,*characterName;
@property(nonatomic,strong) NSSecureTextField* password;
@property(nonatomic,strong) NSTextView* log;
@property(nonatomic,strong) NSTableView *table,*characters;
@property(nonatomic,strong) NSView* creditOverlay;
@property(nonatomic,strong) MuCharacterCreatePanel* createPanel;
@property(nonatomic,strong) MuCharacterMessage* characterMessage;
@property(nonatomic,strong) NSButton* deleteButton;
@property(nonatomic,strong) NSTextField* selectedCharacterLabel;
@property(nonatomic,strong) NSButton *refreshButton,*loginButton,*logoutButton,*reloadButton,*createButton,*selectButton;
@property(nonatomic,strong) NSPopUpButton* classes;
@property(nonatomic,strong) NSTimer* timer;
@end
@implementation ClientController
- (NSTextField*)label:(NSString*)value frame:(NSRect)frame size:(CGFloat)size{
 NSTextField* f=[NSTextField labelWithString:value];f.frame=frame;f.font=[NSFont systemFontOfSize:size];[self.window.contentView addSubview:f];return f;
}
- (NSTableView*)table:(NSRect)frame columns:(NSArray<NSString*>*)columns{
 NSScrollView* scroll=[[NSScrollView alloc] initWithFrame:frame];scroll.hasVerticalScroller=YES;
 NSTableView* t=[[NSTableView alloc] initWithFrame:scroll.bounds];t.dataSource=self;t.delegate=self;
 for(NSString* name in columns){NSTableColumn* c=[[NSTableColumn alloc] initWithIdentifier:name];c.title=name;c.width=frame.size.width/columns.count-8;[t addTableColumn:c];}
 scroll.documentView=t;[self.window.contentView addSubview:scroll];return t;
}
- (void)applicationDidFinishLaunching:(NSNotification*)notification{
 self.itemDrag=[MuItemDrag new];__weak ClientController* transferOwner=self;self.itemDrag.onTransfer=^(mu::client::ItemMove move){[transferOwner transferItem:move];};self.itemDrag.onDrop=^(unsigned slot){[transferOwner groundAction:slot dropping:YES];};__weak ClientController* keyOwner=self;keyMonitor=[NSEvent addLocalMonitorForEventsMatchingMask:(NSEventMaskKeyDown|NSEventMaskFlagsChanged) handler:^NSEvent*(NSEvent* event){
 ClientController* owner=keyOwner;if(!owner||(event.window&&event.window!=owner.window))return event;
 if(event.type==NSEventTypeFlagsChanged){owner->groundNames.flags((event.modifierFlags&NSEventModifierFlagOption)!=0);owner.gameHud.groundNamesLocked=owner->groundNames.latched;return event;}
 if(owner->inWorld&&event.keyCode==49&&!owner.systemMenu.superview){if(event.modifierFlags&NSEventModifierFlagOption){if(!event.isARepeat){owner->groundNames.lock();owner.gameHud.groundNamesLocked=YES;}}else if(!(event.modifierFlags&NSEventModifierFlagCommand)){[owner pickNearestGround];}return nil;}

 if(owner.systemMenu.superview){if(event.keyCode==53){[owner.systemMenu close];return nil;}return event;}
 if(event.keyCode==53){
  if(owner.itemDrag.carrying){[owner.itemDrag cancel];return nil;}
  if(owner.warehousePanel.view.superview){[owner.warehousePanel close];return nil;}
  if(owner.characterInfo.superview){[owner.characterInfo close];return nil;}
  if(owner.inventoryPanel.view.superview){[owner.inventoryPanel close];return nil;}
  if(owner.createPanel&&!owner.createPanel.hidden){if(!owner->working)owner.createPanel.hidden=YES;return nil;}
  if(owner.characterMessage&&!owner.characterMessage.hidden){if(!owner->working&&owner.characterMessage.onCancel)owner.characterMessage.onCancel();return nil;}
  if(owner.creditOverlay.superview){[owner.creditOverlay removeFromSuperview];return nil;}
  [owner toggleSystemMenu:nil];return nil;
 }
 if(owner->inWorld&&!(event.modifierFlags&(NSEventModifierFlagCommand|NSEventModifierFlagOption))){unsigned keys[]={18,19,20,21,23};for(unsigned i=0;i<5;i++)if(event.keyCode==keys[i]){if(event.modifierFlags&NSEventModifierFlagControl)[owner.gameHud assignSkillHotkey:i];else [owner.gameHud selectSkillHotkey:i];return nil;}}
 if(owner->inWorld&&event.keyCode==8&&!(event.modifierFlags&(NSEventModifierFlagCommand|NSEventModifierFlagControl|NSEventModifierFlagOption))){[owner toggleCharacterInfo];return nil;}
 if(owner->inWorld&&(event.keyCode==34||event.keyCode==9)&&!(event.modifierFlags&(NSEventModifierFlagCommand|NSEventModifierFlagControl|NSEventModifierFlagOption))){[owner toggleInventory];return nil;}
 return event;
 }];
 networkQueue=dispatch_queue_create("local.muonline.client.session",DISPATCH_QUEUE_SERIAL);
 self.window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,1100,825) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
 selectedCharacterRow=-1;self.window.releasedWhenClosed=NO;self.window.contentMinSize=NSMakeSize(800,600);self.window.title=@"MU EX603 · Native Mac Client 0.29.7";
 self.log=[[NSTextView alloc] initWithFrame:NSZeroRect];self.log.editable=NO;
 self.status=[self label:@"Loading…" frame:NSMakeRect(30,370,1040,40) size:16];self.status.alignment=NSTextAlignmentCenter;

 NSMenu* bar=[[NSMenu alloc] init];NSMenuItem* item=[[NSMenuItem alloc] init];[bar addItem:item];NSMenu* menu=[[NSMenu alloc] initWithTitle:@"MU Native Client"];
 [menu addItemWithTitle:@"Quit MU Native Client" action:@selector(terminate:) keyEquivalent:@"q"];item.submenu=menu;NSApp.mainMenu=bar;
 NSMenuItem* serverMenu=[menu insertItemWithTitle:@"Select server" action:@selector(logout:) keyEquivalent:@"" atIndex:0];serverMenu.target=self;
 try{NSString* bundled=[[NSBundle mainBundle] pathForResource:@"client" ofType:@"ini"];profile=mu::client::load_profile(bundled?std::string(bundled.UTF8String):std::string());}
 catch(const std::exception& e){self.status.stringValue=text(e.what());return;}
 working=YES;auto current=profile;
 dispatch_async(networkQueue,^{std::string error;try{mu::ui::prepare_lobby_images(current.assetDirectory);}catch(const std::exception& e){error=e.what();}
  dispatch_async(dispatch_get_main_queue(),^{if(!error.empty()){self.status.stringValue=text(error);return;}try{mu::ui::prepare_source_font(profile.assetDirectory,profile.language);[self buildLobby];[self bootstrapData];}catch(const std::exception& e){self.status.stringValue=text(e.what());}});
 });
 self.timer=[NSTimer timerWithTimeInterval:(1./60) target:self selector:@selector(keepalive:) userInfo:nil repeats:YES];[[NSRunLoop mainRunLoop] addTimer:self.timer forMode:NSRunLoopCommonModes];
 [self.window center];[self.window makeKeyAndOrderFront:nil];[NSApp activateIgnoringOtherApps:YES];
}
- (void)credit:(id)sender{
 if(self.creditOverlay){[self.creditOverlay removeFromSuperview];self.creditOverlay=nil;return;}
 try{auto bytes=mu::assets::read_file(std::filesystem::path(profile.assetDirectory)/"Data/Local/Credit.bmd");if(bytes.size()!=400*33)throw std::runtime_error("Unsupported original Credit.bmd layout");constexpr unsigned char key[3]={0xFC,0xCF,0xAB};for(size_t i=0;i<bytes.size();i++)bytes[i]^=key[i%3];NSMutableString* content=[NSMutableString string];for(unsigned i=0;i<400;i++){auto p=bytes.data()+i*33;if(!p[0])break;size_t n=0;while(n<32&&p[1+n])n++;NSString* name=[[NSString alloc] initWithBytes:p+1 length:n encoding:NSUTF8StringEncoding];if(!name)name=[[NSString alloc] initWithBytes:p+1 length:n encoding:NSWindowsCP1252StringEncoding];if(name)[content appendFormat:@"%@\n",name];}
 NSBox* overlay=[[NSBox alloc] initWithFrame:self.lobby.bounds];self.creditOverlay=overlay;overlay.boxType=NSBoxCustom;overlay.fillColor=NSColor.blackColor;overlay.borderWidth=0;overlay.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;[self.lobby addSubview:overlay];NSScrollView* scroll=[[NSScrollView alloc] initWithFrame:NSInsetRect(overlay.bounds,80,70)];scroll.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;scroll.hasVerticalScroller=YES;NSTextView* textView=[[NSTextView alloc] initWithFrame:scroll.contentView.bounds];textView.editable=NO;textView.font=mu::ui::source_font(14);textView.textColor=NSColor.whiteColor;textView.backgroundColor=NSColor.blackColor;textView.string=content;textView.alignment=NSTextAlignmentCenter;textView.autoresizingMask=NSViewWidthSizable;textView.textContainer.widthTracksTextView=YES;scroll.documentView=textView;[overlay addSubview:scroll];NSButton* close=mu_lobby_sprite_button(profile.assetDirectory,"loding_cancel_b_all.tga",@"Close Credit",NSMakeRect(overlay.bounds.size.width-84,20,54,30),self,@selector(credit:));close.autoresizingMask=NSViewMinXMargin|NSViewMaxYMargin;[overlay addSubview:close];
 }catch(const std::exception& e){self.status.hidden=NO;self.status.stringValue=text(e.what());}
}
- (void)bootstrapData{
 [self busy:YES];[self.lobby setStage:mu::ui::LobbyStage::Boot];[self.lobby setCompleted:0 total:13000];self.status.stringValue=@"Loading Data…";auto current=profile;
 dispatch_async(networkQueue,^{std::string error;const auto started=std::chrono::steady_clock::now();std::ofstream trace("/tmp/mu-client-bootstrap.log",std::ios::trunc);trace<<"source=local_Data groups=13\n";auto progress=[&](unsigned step){trace<<"completed="<<step<<" elapsed_ms="<<std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count()<<"\n";trace.flush();dispatch_async(dispatch_get_main_queue(),^{[self.lobby setCompleted:step*1000 total:13000];});};
  try{mu::ui::load_item_catalog(std::filesystem::path(current.assetDirectory)/"Data",current.language);progress(1);mu::ui::preload_item_panel_assets(current.assetDirectory,current.language);progress(2);
   mu::viewer::load(current.assetDirectory,0,135,124,0,0,[&](unsigned step,unsigned total){progress(2+step);});
   lobbyScene=mu::lobby::load(current.assetDirectory,[&](unsigned done,unsigned total){unsigned units=10000+(total?done*1000/total:0);trace<<"lobby_models="<<done<<"/"<<total<<"\n";dispatch_async(dispatch_get_main_queue(),^{[self.lobby setCompleted:units total:13000];});});
   characterScene=mu::lobby::load(current.assetDirectory,[&](unsigned done,unsigned total){unsigned units=11000+(total?done*1000/total:0);dispatch_async(dispatch_get_main_queue(),^{[self.lobby setCompleted:units total:13000];});},75);
   creationPreviews=mu::lobby::load_creation_previews(current.assetDirectory);progress(13);
   trace<<"lobby_world="<<lobbyScene->world<<" placements="<<lobbyScene->placements.objects.size()<<" model_types="<<lobbyScene->models.size()<<" missing_model_types=";for(auto type:lobbyScene->missingModelTypes)trace<<type<<",";trace<<"\n";
  }catch(const std::exception& e){error=e.what();}
  dispatch_async(dispatch_get_main_queue(),^{if(!error.empty()){self.status.hidden=NO;self.status.stringValue=text(error);[self append:text(error)];return;}[self.lobby attachScene:lobbyScene];[self.lobby attachCharacterScene:characterScene];[self installCreationPanel];[self.lobby setCompleted:13000 total:13000];[self.lobby displayIfNeeded];
   // Presentation dwell only: data is already complete and stays at 100%.
   // No timer advances loading progress or delays disk/network work.
   double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();double remaining=std::max(0.,1.5-elapsed);
   dispatch_after(dispatch_time(DISPATCH_TIME_NOW,int64_t(remaining*NSEC_PER_SEC)),dispatch_get_main_queue(),^{[self busy:NO];[self refresh:nil];});});
 });
}
- (void)buildLobby{
 self.lobby=[[MuLobbyView alloc] initWithRoot:profile.assetDirectory language:profile.language];self.lobby.frame=self.window.contentView.bounds;self.window.contentView=self.lobby;self.status=self.lobby.status;
 auto label=[&](NSView* parent,NSString* value,NSRect rect){auto f=[NSTextField labelWithString:value];f.frame=rect;f.font=mu::ui::source_font(12);f.textColor=[NSColor colorWithRed:1 green:.9 blue:.75 alpha:1];[parent addSubview:f];return f;};
 auto button=[&](NSView* parent,NSString* value,NSRect rect,SEL action){auto b=[NSButton buttonWithTitle:value target:self action:action];b.frame=rect;b.font=mu::ui::source_font(12);[parent addSubview:b];return b;};
 auto field=[&](NSView* parent,NSRect rect,BOOL secure){NSTextField* f=secure?[[NSSecureTextField alloc] initWithFrame:rect]:[[NSTextField alloc] initWithFrame:rect];f.font=mu::ui::source_font(12);f.textColor=[NSColor colorWithRed:1 green:.9 blue:.82 alpha:1];f.backgroundColor=[NSColor colorWithWhite:0 alpha:.7];[parent addSubview:f];return f;};
 auto table=[&](NSView* parent,NSRect rect,NSArray<NSString*>* columns){auto t=[self table:rect columns:columns];NSView* scroll=t.enclosingScrollView;[parent addSubview:scroll];t.backgroundColor=[NSColor colorWithWhite:0 alpha:.75];t.rowHeight=29;t.headerView=nil;t.gridStyleMask=NSTableViewSolidHorizontalGridLineMask;t.gridColor=[NSColor colorWithWhite:.3 alpha:.8];for(NSTableColumn* c in t.tableColumns){((NSTextFieldCell*)c.dataCell).font=mu::ui::source_font(12);((NSTextFieldCell*)c.dataCell).textColor=[NSColor colorWithRed:1 green:.9 blue:.75 alpha:1];}return t;};
 NSView* v=self.lobby.serverControls;
 self.table=[[NSTableView alloc] init];self.table.dataSource=self;self.table.delegate=self;
 self.serverSelector=[[MuServerSelector alloc] initWithRoot:profile.assetDirectory language:profile.language];self.serverSelector.frame=v.bounds;self.serverSelector.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;[v addSubview:self.serverSelector];__weak ClientController* selectorOwner=self;self.serverSelector.onChoose=^(unsigned row){ClientController* owner=selectorOwner;if(!owner)return;[owner.table selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];[owner connectServer:nil];};
 NSButton* mainMenu=mu_lobby_sprite_button(profile.assetDirectory,"server_menu_b_all.tga",@"Menu",NSMakeRect(30,v.bounds.size.height-60,54,30),self,@selector(toggleSystemMenu:));mainMenu.autoresizingMask=NSViewMinYMargin;[v addSubview:mainMenu];
 NSButton* credit=mu_lobby_sprite_button(profile.assetDirectory,"server_credit_b_all.tga",@"Credit",NSMakeRect(v.bounds.size.width-84,v.bounds.size.height-60,54,30),self,@selector(credit:));credit.autoresizingMask=NSViewMinXMargin|NSViewMinYMargin;[v addSubview:credit];
 // Endpoint belongs in client.ini, not in the original game's login flow.
 self.host=[[NSTextField alloc] init];self.host.stringValue=text(profile.connectHost);self.port=[[NSTextField alloc] init];self.port.stringValue=[NSString stringWithFormat:@"%d",profile.connectPort];
 v=self.lobby.loginControls;
 label(v,[self.lobby globalText:450 fallback:@"Account"],NSMakeRect(28,109,75,20));
 label(v,[self.lobby globalText:451 fallback:@"Password"],NSMakeRect(28,134,75,20));
 self.account=field(v,NSMakeRect(115,110,140,18),NO);self.password=(NSSecureTextField*)field(v,NSMakeRect(115,135,140,18),YES);
 self.account.bordered=NO;self.password.bordered=NO;self.account.focusRingType=NSFocusRingTypeNone;self.password.focusRingType=NSFocusRingTypeNone;self.account.nextKeyView=self.password;self.password.nextKeyView=self.account;
 self.password.target=self;self.password.action=@selector(login:);
 self.loginButton=mu_lobby_sprite_button(profile.assetDirectory,"message_ok_b_all.tga",[self.lobby globalText:228 fallback:@"OK"],NSMakeRect(150,178,54,30),self,@selector(login:));[v addSubview:self.loginButton];self.loginButton.keyEquivalent=@"\r";
 self.cancelLoginButton=mu_lobby_sprite_button(profile.assetDirectory,"loding_cancel_b_all.tga",[self.lobby globalText:229 fallback:@"Cancel"],NSMakeRect(211,178,54,30),self,@selector(logout:));[v addSubview:self.cancelLoginButton];
 v=self.lobby.characterControls;
 self.characters=[[NSTableView alloc] init];self.characters.dataSource=self;self.characters.delegate=self;
 __weak ClientController* characterOwner=self;self.lobby.onCharacter=^(unsigned row,BOOL enter){ClientController* owner=characterOwner;if(!owner||owner->working||!owner->authenticated||owner->inWorld||row>=owner->characterList.characters.size())return;owner->selectedCharacterRow=row;const auto& c=owner->characterList.characters[row];owner.selectedCharacterLabel.stringValue=[NSString stringWithFormat:@"%@ · %u",text(c.name),c.level];[owner busy:NO];if(enter)[owner select:nil];};
 self.createButton=mu_lobby_sprite_button(profile.assetDirectory,"b_create.tga",@"Create",NSMakeRect(30,540,54,30),self,@selector(toggleCreate:));[v addSubview:self.createButton];
 self.backButton=mu_lobby_sprite_button(profile.assetDirectory,"server_menu_b_all.tga",@"Menu",NSMakeRect(85,540,54,30),self,@selector(toggleSystemMenu:));[v addSubview:self.backButton];
 self.selectButton=mu_lobby_sprite_button(profile.assetDirectory,"b_connect.tga",@"Connect",NSMakeRect(661,540,54,30),self,@selector(select:));[v addSubview:self.selectButton];self.selectButton.keyEquivalent=@"\r";
 self.deleteButton=mu_lobby_sprite_button(profile.assetDirectory,"b_delete.tga",@"Delete",NSMakeRect(716,540,54,30),self,@selector(deleteCharacter:));[v addSubview:self.deleteButton];
 self.selectedCharacterLabel=label(v,@"",NSMakeRect(145,545,505,21));self.selectedCharacterLabel.alignment=NSTextAlignmentCenter;
 self.createButton.autoresizingMask=NSViewMinYMargin;self.backButton.autoresizingMask=NSViewMinYMargin;self.selectButton.autoresizingMask=NSViewMinXMargin|NSViewMinYMargin;self.deleteButton.autoresizingMask=self.selectButton.autoresizingMask;self.selectedCharacterLabel.autoresizingMask=NSViewWidthSizable|NSViewMinYMargin;
 self.logoutButton=self.backButton;[self busy:NO];
}
- (void)installCreationPanel{
 self.createPanel=[[MuCharacterCreatePanel alloc] initWithRoot:profile.assetDirectory language:profile.language previews:creationPreviews];self.createPanel.frame=self.lobby.bounds;[self.lobby addSubview:self.createPanel];self.characterName=self.createPanel.nameField;__weak ClientController* weak=self;self.createPanel.onCreate=^{[weak create:nil];};self.createPanel.onCancel=^{weak.createPanel.hidden=YES;};
 self.characterMessage=[[MuCharacterMessage alloc] initWithRoot:profile.assetDirectory];self.characterMessage.frame=self.lobby.bounds;[self.lobby addSubview:self.characterMessage];self.characterMessage.onCancel=^{weak.characterMessage.hidden=YES;};
}
- (void)showCharacterMessage:(NSString*)message{__weak ClientController* weak=self;self.characterMessage.onOK=^{weak.characterMessage.hidden=YES;};self.characterMessage.onCancel=^{weak.characterMessage.hidden=YES;};[self.characterMessage showText:message input:NO confirm:NO];}
- (void)deleteCharacter:(id)sender{
 if(working||!authenticated||inWorld||selectedCharacterRow<0||size_t(selectedCharacterRow)>=characterList.characters.size())return;auto c=characterList.characters[selectedCharacterRow];if(c.guildStatus!=255){[self showCharacterMessage:[self.lobby globalText:1654 fallback:@""]];return;}if(c.control&18){[self showCharacterMessage:[self.lobby globalText:439 fallback:@""]];return;}if(!characterList.deletePolicyReady||c.level>=characterList.deleteMaxLevel){NSString* message=[self.lobby globalText:1711 fallback:@""];message=[message stringByReplacingOccurrencesOfString:@"%d" withString:[NSString stringWithFormat:@"%u",characterList.deleteMaxLevel]];[self showCharacterMessage:message];return;}
 pendingDeleteName=c.name;NSString* prompt=[[self.lobby globalText:1712 fallback:@""] stringByReplacingOccurrencesOfString:@"%s" withString:text(c.name)];__weak ClientController* weak=self;self.characterMessage.onOK=^{[weak deleteCode:nil];};self.characterMessage.onCancel=^{weak.characterMessage.hidden=YES;};[self.characterMessage showText:prompt input:NO confirm:YES];
}
- (void)deleteCode:(id)sender{__weak ClientController* weak=self;self.characterMessage.onOK=^{[weak submitDelete:nil];};[self.characterMessage showText:[self.lobby globalText:1713 fallback:@""] input:YES confirm:YES];}
- (void)submitDelete:(id)sender{
 if(working||!authenticated||inWorld||pendingDeleteName.empty())return;auto name=pendingDeleteName;auto code=std::make_shared<std::string>(self.characterMessage.codeField.stringValue.UTF8String);self.characterMessage.codeField.stringValue=@"";[self busy:YES];[self.characterMessage showText:[self.lobby globalText:471 fallback:@""] input:NO confirm:NO];[self.characterMessage setWaiting:YES];
 dispatch_async(networkQueue,^{auto list=std::make_shared<mu::client::CharacterList>();unsigned result=255;std::string error;try{if(!session)throw std::runtime_error("Session disconnected");result=session->erase(name,*code);*list=session->characters();}catch(const std::exception& e){error=e.what();}std::fill(code->begin(),code->end(),0);dispatch_async(dispatch_get_main_queue(),^{pendingDeleteName.clear();if(error.empty()){characterList=*list;[self refreshCharacterVisuals];}else [self append:text(error)];[self busy:NO];unsigned id=result==1?1714:result==0?1654:result==3?439:401;[self showCharacterMessage:error.empty()?[self.lobby globalText:id fallback:@""]:text(error)];});});
}
- (void)toggleCreate:(id)sender{if(working)return;[self.createPanel configureClassMask:characterList.classCode];self.createPanel.hidden=!self.createPanel.hidden;if(!self.createPanel.hidden)[self.window makeFirstResponder:self.characterName];}
- (void)refreshCharacterVisuals{
 selectedCharacterRow=-1;[self.lobby setCharacters:{}];auto list=characterList;auto root=profile.assetDirectory;unsigned revision=++characterVisualRevision;self.createPanel.hidden=YES;if(!authenticated)return;self.selectedCharacterLabel.stringValue=@"";
 dispatch_async(networkQueue,^{std::vector<mu::lobby::CharacterVisual> visuals;std::string error;try{visuals=mu::lobby::load_characters(root,list);}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{if(revision!=characterVisualRevision||!authenticated)return;if(!error.empty()){[self append:text(error)];self.status.hidden=NO;self.status.stringValue=text(error);return;}[self.lobby setCharacters:visuals];for(auto& entry:visuals)for(auto& limitation:entry.unsupported)[self append:text(entry.character.name+": "+limitation+" not ported")];});});
}
- (void)connectServer:(id)sender{
 if(working||authenticated||gameConnected)return;NSInteger row=self.table.selectedRow;if(row<0||std::size_t(row)>=servers.size())return;
 auto current=profile;auto code=servers[row].code;[self busy:YES];[self.lobby setStage:mu::ui::LobbyStage::Connecting];self.status.stringValue=@"Connecting to GameServer…";
 dispatch_async(networkQueue,^{std::string error;try{session=std::make_shared<mu::client::Session>(current,code);__weak ClientController* weak=self;session->setNoticeHandler([weak](const mu::client::ServerNotice& notice){auto copy=notice;dispatch_async(dispatch_get_main_queue(),^{ClientController* owner=weak;if(!owner)return;[owner.gameHud addNotice:copy];if(!owner->inWorld)[owner append:text(copy.message)];});});}catch(const std::exception& e){error=e.what();session.reset();}
  dispatch_async(dispatch_get_main_queue(),^{gameConnected=error.empty();[self busy:NO];self.status.stringValue=error.empty()?@"":text(error);if(gameConnected)[self.window makeFirstResponder:self.account];});
 });
}
- (void)returnToLobby{hasPendingLoot=NO;lastCastRevision=lastDamageRevision=lastLevelRevision=0;casting=NO;[self.characterInfo close];self.characterInfo=nil;[self.gameHud stop];self.gameHud=nil;
 [self.systemMenu close];
 [self.itemDrag cancel];self.inventoryPanel.onClose=nil;self.warehousePanel.onClose=nil;[self.inventoryPanel close];[self.warehousePanel close];self.inventoryPanel=nil;self.warehousePanel=nil;
 [self.worldViewer stop];self.worldViewer=nil;onlineScene.reset();route.clear();routeRequested=NO;hasCombatTarget=NO;npcWaiting=NO;npcOpen=NO;moving=NO;
 self.lobby.frame=self.window.contentView.bounds;self.window.contentView=self.lobby;self.window.title=authenticated?@"MU EX603 · Select character":@"MU EX603 · Select server";self.password.stringValue=@"";[self.serverSelector resetSelection];
}
- (void)append:(NSString*)value{self.log.string=[self.log.string stringByAppendingFormat:@"%@\n",value];[self.log scrollRangeToVisible:NSMakeRange(self.log.string.length,0)];}
- (void)busy:(BOOL)value{
 working=value;self.serverSelector.enabled=!value&&!authenticated&&!gameConnected;self.cancelLoginButton.enabled=!value;if(!value&&!inWorld)[self.lobby setStage:authenticated?mu::ui::LobbyStage::Characters:(gameConnected?mu::ui::LobbyStage::Login:mu::ui::LobbyStage::Servers)];self.connectButton.enabled=!value&&!authenticated&&!gameConnected&&self.table.selectedRow>=0;self.backButton.enabled=!value;self.refreshButton.enabled=!value&&!authenticated;self.host.enabled=!value&&!authenticated;self.port.enabled=!value&&!authenticated;
 self.table.enabled=!value&&!authenticated;self.characters.enabled=!value&&authenticated&&!inWorld;
 self.loginButton.enabled=!value&&!authenticated&&gameConnected;self.logoutButton.enabled=!value&&authenticated;
 self.account.enabled=!value&&!authenticated;self.password.enabled=!value&&!authenticated;
 self.createButton.enabled=!value&&authenticated&&!inWorld&&characterList.characters.size()<5;[self.createPanel setBusy:value];self.deleteButton.enabled=!value&&authenticated&&!inWorld&&selectedCharacterRow>=0;self.reloadButton.enabled=!value&&authenticated&&!inWorld;
 self.selectButton.enabled=!value&&authenticated&&!inWorld&&selectedCharacterRow>=0;self.characterName.enabled=!value&&authenticated&&!inWorld;self.classes.enabled=!value&&authenticated&&!inWorld;
}
- (BOOL)updateEndpoint{
 try{auto trim=[](std::string v){auto first=v.find_first_not_of(" \t\r\n");return first==std::string::npos?std::string():v.substr(first,v.find_last_not_of(" \t\r\n")-first+1);};std::string v=trim(self.port.stringValue.UTF8String);std::size_t end=0;int port=std::stoi(v,&end);if(end!=v.size()||port<1||port>65535)throw std::runtime_error("Port must be 1..65535");profile.connectPort=port;profile.connectHost=trim(self.host.stringValue.UTF8String);self.host.stringValue=text(profile.connectHost);self.port.stringValue=[NSString stringWithFormat:@"%d",port];return YES;}
 catch(const std::exception& e){[self append:text(e.what())];return NO;}
}
- (void)refresh:(id)sender{
 if(authenticated||gameConnected)return;
 if(![self updateEndpoint])return;[self busy:YES];self.status.stringValue=@"Đang lấy danh sách server…";auto current=profile;
 dispatch_async(networkQueue,^{auto rows=std::make_shared<std::vector<mu::client::Server>>();std::string error;
  try{*rows=mu::client::discover(current);}catch(const std::exception& e){error=e.what();}
  dispatch_async(dispatch_get_main_queue(),^{servers=*rows;[self.serverSelector setServers:servers];[self.table reloadData];[self.table deselectAll:nil];[self busy:NO];self.status.hidden=error.empty();self.status.stringValue=error.empty()?@"":@"Chưa lấy được danh sách";[self append:error.empty()?@"PASS: ConnectServer discovery.":text(error)];});
 });
}
- (void)login:(id)sender{
 if(working||authenticated||!gameConnected)return;
std::string account=self.account.stringValue.UTF8String,password=self.password.stringValue.UTF8String;self.password.stringValue=@"";
 [self busy:YES];[self.lobby setStage:mu::ui::LobbyStage::Authenticating];self.status.stringValue=@"Đang đăng nhập…";
 dispatch_async(networkQueue,^{auto list=std::make_shared<mu::client::CharacterList>();std::string message;bool success=false,connected=true;
  try{if(!session)throw std::runtime_error("Session disconnected");unsigned result=session->login(account,password);message=mu::client::login_result(result);if(result==1){*list=session->characters();success=true;}}
  catch(const std::exception& e){message=e.what();session.reset();connected=false;}
  dispatch_async(dispatch_get_main_queue(),^{authenticated=success;gameConnected=connected;characterList=*list;[self.characters reloadData];[self refreshCharacterVisuals];selectedCharacterRow=-1;self.status.stringValue=text(message);[self append:text(message)];if(success)[self append:[NSString stringWithFormat:@"PASS: danh sách nhân vật từ server (%lu nhân vật).",(unsigned long)characterList.characters.size()]];[self busy:NO];});
 });
 std::fill(password.begin(),password.end(),0);
}
- (void)reload:(id)sender{
 if(working||!authenticated||inWorld)return;
 [self busy:YES];dispatch_async(networkQueue,^{auto list=std::make_shared<mu::client::CharacterList>();std::string error;
  try{if(!session)throw std::runtime_error("Session disconnected");*list=session->characters();}catch(const std::exception& e){error=e.what();session.reset();}
  dispatch_async(dispatch_get_main_queue(),^{if(!error.empty()){authenticated=NO;gameConnected=NO;[self append:text(error)];self.status.stringValue=@"Phiên kết nối đã đóng";}else{characterList=*list;self.status.stringValue=@"Đã tải danh sách nhân vật";}[self.characters reloadData];[self refreshCharacterVisuals];[self busy:NO];});
 });
}
- (void)create:(id)sender{
 if(working||!authenticated||inWorld)return;
 std::string name=self.characterName.stringValue.UTF8String;if(name.size()<4||name.size()>10){[self showCharacterMessage:[self.lobby globalText:390 fallback:@""]];return;}for(unsigned char c:name)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9'))){[self showCharacterMessage:[self.lobby globalText:391 fallback:@""]];return;}unsigned cls=self.createPanel.selectedClass*16;
 [self busy:YES];self.createPanel.hidden=YES;[self.characterMessage showText:[self.lobby globalText:471 fallback:@""] input:NO confirm:NO];[self.characterMessage setWaiting:YES];
 dispatch_async(networkQueue,^{auto list=std::make_shared<mu::client::CharacterList>();std::string message;bool connection=true;unsigned created=255;
  try{if(!session)throw std::runtime_error("Session disconnected");unsigned result=session->create(name,cls);created=result;message=result==1?"Đã tạo nhân vật":("Server từ chối tạo nhân vật (code "+std::to_string(result)+"). Kiểm tra tên trùng, slot hoặc điều kiện mở class.");*list=session->characters();}
  catch(const std::exception& e){message=e.what();session.reset();connection=false;}
  dispatch_async(dispatch_get_main_queue(),^{authenticated=connection;gameConnected=connection;characterList=*list;[self.characters reloadData];[self refreshCharacterVisuals];self.status.stringValue=text(message);[self append:text(message)];[self busy:NO];if(created==1)self.characterMessage.hidden=YES;else{if(connection){[self.createPanel configureClassMask:characterList.classCode];self.createPanel.hidden=NO;}[self showCharacterMessage:created==0?[self.lobby globalText:1716 fallback:@""]:created==2?[self.lobby globalText:396 fallback:@""]:text(message)];}});
 });
}
- (void)select:(id)sender{
 if(working||!authenticated||inWorld)return;NSInteger row=selectedCharacterRow;if(row<0||std::size_t(row)>=characterList.characters.size())return;auto name=characterList.characters[row].name;auto current=profile;
 [self busy:YES];[self.lobby setStage:mu::ui::LobbyStage::Loading];self.status.stringValue=@"Đang vào map và tải cảnh…";
 dispatch_async(networkQueue,^{std::string message;std::shared_ptr<mu::viewer::SceneData> data;mu::client::WorldState state;
  try{if(!session)throw std::runtime_error("Session disconnected");auto world=session->select(name);
   state=session->pump();auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(!state.inventoryReady&&std::chrono::steady_clock::now()<until){std::this_thread::sleep_for(std::chrono::milliseconds(10));state=session->pump();}if(!state.inventoryReady)throw std::runtime_error("Authoritative character equipment was not received");auto resources=mu::viewer::character_resources(world.character,state.inventory.get());activeCharacter=world.character;equipmentKey=resources.key();data=mu::viewer::load(current.assetDirectory,world.map,world.x,world.y,world.direction,world.baseClass,{},&resources);data->movementRate=current.movementRate;data->mapName=mu::ui::load_texts(std::filesystem::path(current.assetDirectory)/"Data",current.language).at(world.map==51?1853:30+world.map);mu::ui::preload_item_panel_assets(current.assetDirectory,current.language);state=session->pump();iconInventoryRevision=0;iconNpcItems.reset();if(state.inventory){mu::ui::prepare_item_icons(current.assetDirectory,current.language,state.inventory->items);iconInventoryRevision=state.inventory->revision;}message="PASS: world-entry; đang giữ phiên game thật.";
  }catch(const std::exception& e){message=e.what();session.reset();}
  dispatch_async(dispatch_get_main_queue(),^{authenticated=bool(data);gameConnected=bool(data);inWorld=bool(data);onlineScene=data;[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];self.status.stringValue=data?text(data->mapName):@"Không vào được map";[self append:text(message)];[self busy:NO];if(data){try{
   [self.lobby setStage:mu::ui::LobbyStage::World];self.itemDrag.baseClass=data->assets->characterResources.baseClass;[self.worldViewer stop];self.worldViewer=[[MuWorldViewer alloc] init];[self.worldViewer showScene:data inWindow:self.window];[self.worldViewer updateViewport:state.viewport];
   [self.gameHud stop];self.gameHud=[[MuGameHud alloc] initWithRoot:profile.assetDirectory language:profile.language scene:self.worldViewer.sceneView actor:self.worldViewer.actorNode name:characterList.characters[selectedCharacterRow].name];[self.worldViewer.interfaceView addSubview:self.gameHud];[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];__weak ClientController* hudOwner=self;self.gameHud.onInventory=^{[hudOwner toggleInventory];};self.gameHud.onCharacter=^{[hudOwner toggleCharacterInfo];};
   __weak ClientController* owner=self;self.itemDrag.dropView=self.worldViewer.sceneView;self.worldViewer.onAreaSkill=^BOOL{if(owner.gameHud.activeSkill!=9)return NO;[owner castEvilSpirit];return YES;};self.worldViewer.onLoot=^(unsigned index){[owner pickGround:index];};self.worldViewer.onMove=^(unsigned direction){[owner keyboardMoveDirection:direction];};self.worldViewer.onClose=^{[owner logout:nil];};self.worldViewer.onDestination=^(unsigned x,unsigned y){[owner destinationX:x y:y];};self.worldViewer.onEntity=^(unsigned index){[owner talkNpc:index];};self.worldViewer.onSkillTarget=^(unsigned index){[owner requestCombat:index skill:YES];};
   [self.worldViewer setMessage:@"Online · click terrain để đi, hoặc giữ mũi tên · đóng cửa sổ để logout"];
  }catch(const std::exception& e){authenticated=gameConnected=inWorld=NO;auto error=text(e.what());[self returnToLobby];self.status.stringValue=error;[self append:error];dispatch_async(networkQueue,^{session.reset();});}}});
 });
}
- (void)transferItem:(mu::client::ItemMove)move{
 if(self.itemDrag.pending||!authenticated||!inWorld||working)return;self.itemDrag.pending=YES;route.clear();routeRequested=NO;[self.worldViewer setMessage:@"Đang chờ server xác nhận chuyển đồ…"];
 dispatch_async(networkQueue,^{mu::client::WorldState state;std::string error,resourceError;std::shared_ptr<const mu::viewer::CharacterBody> body;bool accepted=false;try{if(!session)throw std::runtime_error("Session disconnected");accepted=session->moveItem(move);state=session->state();if(accepted)try{body=changedEquipment(profile,activeCharacter,state,equipmentKey);}catch(const std::exception& e){resourceError=e.what();}}catch(const std::exception& e){error=e.what();session.reset();}dispatch_async(dispatch_get_main_queue(),^{self.itemDrag.pending=NO;[self.itemDrag cancel];if(!error.empty()){authenticated=NO;gameConnected=NO;inWorld=NO;[self returnToLobby];self.status.stringValue=text(error);[self append:text(error)];[self busy:NO];return;}if(!resourceError.empty())[self append:text("Equipment resource: "+resourceError)];if(body)try{[self.worldViewer updateEquipment:body];}catch(const std::exception& e){[self append:text(e.what())];}[self.worldViewer setBootsLevel:state.bootsLevel];[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];[self.inventoryPanel updateState:state];[self.warehousePanel updateState:state.npc];[self.worldViewer setMessage:accepted?@"Server đã xác nhận chuyển đồ":@"Server từ chối chuyển đồ; vị trí cũ được giữ nguyên"];if(self.logoutAfterTransfer){self.logoutAfterTransfer=NO;[self logout:nil];}});});
}
- (void)toggleCharacterInfo{
 if(!inWorld||working)return;
 if(self.characterInfo.superview){[self.characterInfo close];return;}
 try{if(!self.characterInfo)self.characterInfo=[[MuCharacterInfo alloc] initWithRoot:profile.assetDirectory language:profile.language name:activeCharacter.name];__weak ClientController* owner=self;self.characterInfo.onAddStat=^(unsigned type){ClientController* strong=owner;if(!strong)return;dispatch_async(strong->networkQueue,^{mu::client::WorldState next;std::string error;bool accepted=false;try{if(!strong->session)throw std::runtime_error("Session disconnected");accepted=strong->session->addStat(type);next=strong->session->state();}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{strong.characterInfo.pending=NO;if(error.empty()){strong->latestState=next;[strong.characterInfo updateState:next];[strong.gameHud updateState:next];[strong.itemDrag setAttributes:next.attributes];}else [strong.worldViewer setMessage:text(error)];});});};[self.characterInfo updateState:latestState];[self.characterInfo attach:self.worldViewer.interfaceView];}catch(const std::exception& e){[self.worldViewer setMessage:text(e.what())];}
}
- (void)toggleInventory{
 if(npcOpen&&self.warehousePanel.view.superview){[self.warehousePanel close];return;}if(!inWorld||working||npcOpen||npcWaiting)return;
 if(self.inventoryPanel.view.superview){[self.inventoryPanel close];return;}
 if(!latestState.inventory||!latestState.inventory->ready){[self.worldViewer setMessage:@"Đang chờ inventory từ server"];return;}
 try{if(!self.inventoryPanel){self.inventoryPanel=[[MuInventoryPanel alloc] initWithRoot:profile.assetDirectory language:profile.language];self.inventoryPanel.dragController=self.itemDrag;__weak ClientController* owner=self;self.inventoryPanel.onClose=^{ClientController* strong=owner;if(strong.warehousePanel.view.superview)[strong.warehousePanel close];else [strong.worldViewer setInterfaceColumns:0];};}__weak ClientController* bookOwner=self;self.inventoryPanel.onUseBook=^(unsigned slot){[bookOwner shopAction:3 slot:slot];};[self.inventoryPanel updateState:latestState];[self.inventoryPanel attachToGameView:self.worldViewer.interfaceView];[self.worldViewer setInterfaceColumns:1];[self.worldViewer setMessage:@"Inventory · click để nhấc/đặt · Esc để trả item hoặc đóng"];}catch(const std::exception& e){[self.worldViewer setMessage:text(e.what())];}
}
- (void)talkNpc:(unsigned)index{
 if(latestState.viewport){auto e=latestState.viewport->entities.find(index);if(e!=latestState.viewport->entities.end()&&e->second.kind==mu::client::EntityKind::Monster&&onlineScene&&onlineScene->assets->actors.count(e->second.type)&&!onlineScene->assets->actors.at(e->second.type)->npc){[self requestCombat:index skill:NO];return;}}
 if(self.itemDrag.pending||!inWorld||working||moving||npcWaiting||npcOpen||[self.worldViewer pathPending]||!latestState.viewport)return;
 auto found=latestState.viewport->entities.find(index);if(found==latestState.viewport->entities.end())return;
 auto visual=onlineScene->assets->actors.find(found->second.type);if(visual==onlineScene->assets->actors.end()||!visual->second->npc){[self attackMonster:index skill:NO];return;}
 auto tile=[self.worldViewer movementTile];if(std::hypot(double(tile.first)-found->second.x,double(tile.second)-found->second.y)>5){[self.worldViewer setMessage:@"Đi tới gần NPC (trong 5 ô) rồi click lại"];return;}
 route.clear();routeRequested=NO;npcWaiting=YES;npcIndex=index;npcStarted=[NSProcessInfo processInfo].systemUptime;[self.worldViewer setMessage:@"Đang chờ phản hồi NPC từ server…"];
 dispatch_async(networkQueue,^{try{session->talk(index);}catch(const std::exception& e){auto error=text(e.what());dispatch_async(dispatch_get_main_queue(),^{npcWaiting=NO;[self.worldViewer setMessage:error];});}});
}
- (void)castEvilSpirit{
 if(!inWorld||working||casting||moving||npcOpen||npcWaiting||self.systemMenu.superview||self.itemDrag.carrying||[self.worldViewer pathPending])return;double now=NSProcessInfo.processInfo.systemUptime;if(now-lastCastSent<.6)return;lastCastSent=now;casting=YES;route.clear();routeRequested=NO;hasCombatTarget=NO;hasPendingLoot=NO;auto direction=latestState.direction;
 dispatch_async(networkQueue,^{std::string error;mu::client::WorldState state;try{if(!session)throw std::runtime_error("Session disconnected");session->castAreaSkill(9,direction);state=session->pump();}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{casting=NO;if(!error.empty()){[self.worldViewer setMessage:text(error)];return;}[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.worldViewer updateViewport:state.viewport];});});
}
- (void)pickNearestGround{if(!inWorld||working||moving||casting||self.itemDrag.carrying||npcOpen||self.systemMenu.superview||!latestState.viewport)return;auto tile=[self.worldViewer movementTile];double nearest=2.01;unsigned chosen=UINT_MAX;for(const auto& pair:latestState.viewport->items){double distance=std::hypot(double(tile.first)-pair.second.x,double(tile.second)-pair.second.y);if(distance<nearest){nearest=distance;chosen=pair.first;}}if(chosen!=UINT_MAX)[self pickGround:chosen];}
- (void)pickGround:(unsigned)index{if(!inWorld||working||moving||casting||self.itemDrag.carrying||npcOpen||!latestState.viewport)return;auto it=latestState.viewport->items.find(index);if(it==latestState.viewport->items.end())return;auto tile=[self.worldViewer movementTile];if(std::hypot(double(tile.first)-it->second.x,double(tile.second)-it->second.y)>2){[self destinationX:it->second.x y:it->second.y];pendingLoot=index;hasPendingLoot=YES;return;}hasPendingLoot=NO;[self groundAction:index dropping:NO];}
- (void)groundAction:(unsigned)value dropping:(BOOL)dropping{
 if(!inWorld||working||moving||casting||self.itemDrag.pending||npcOpen)return;self.itemDrag.pending=YES;hasPendingLoot=NO;route.clear();routeRequested=NO;
 dispatch_async(networkQueue,^{std::string error;bool accepted=false;mu::client::WorldState state;try{if(!session)throw std::runtime_error("Session disconnected");accepted=dropping?session->dropItem(value):session->pickItem(value);state=session->pump();if(state.inventory)mu::ui::prepare_item_icons(profile.assetDirectory,profile.language,state.inventory->items);}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{self.itemDrag.pending=NO;[self.itemDrag cancel];if(!error.empty()){[self.worldViewer setMessage:text(error)];return;}if(accepted)mu::audio::play(@"pDropItem.wav",.7);[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.inventoryPanel updateState:state];[self.worldViewer updateViewport:state.viewport];[self.worldViewer setMessage:accepted?@"":@"Server từ chối thao tác nhặt/quăng đồ"];});});
}
- (void)applyCombat:(const mu::client::WorldState&)state{
 for(const auto& hit:state.combat.hits)if(hit.revision>lastDamageRevision){if(hasCombatTarget&&hit.target==pendingCombatTarget)hasCombatTarget=NO;}
 if(state.combat.levelRevision!=lastLevelRevision){lastLevelRevision=state.combat.levelRevision;[self.worldViewer playLevelUp];}
 if(state.combat.castRevision!=lastCastRevision){lastCastRevision=state.combat.castRevision;[self.worldViewer setCastRevision:lastCastRevision];[self.worldViewer playSpell:state.combat.skill];[self.worldViewer playSkillEffect:state.combat.skill target:state.combat.castTarget viewport:state.viewport];}
 if(state.combat.damageRevision!=lastDamageRevision){lastDamageRevision=state.combat.damageRevision;if(state.viewport&&state.viewport->entities.count(state.combat.target)){auto& target=state.viewport->entities.at(state.combat.target);if(target.kind==mu::client::EntityKind::Monster)[self.worldViewer setMessage:[NSString stringWithFormat:@"Sát thương server: %u · EXP: %u",state.combat.damage,state.vitals.experience]];}}
}
- (void)requestCombat:(unsigned)index skill:(BOOL)useSkill{if(!onlineScene||!latestState.viewport)return;auto target=latestState.viewport->entities.find(index);if(target==latestState.viewport->entities.end())return;auto visual=onlineScene->assets->actors.find(target->second.type);if(visual==onlineScene->assets->actors.end()||visual->second->npc)return;route.clear();routeRequested=NO;pendingCombatTarget=index;pendingCombatSkill=useSkill;combatAttempts=0;hasCombatTarget=YES;[self continueCombat];}
- (void)continueCombat{
 if(!hasCombatTarget||working||self.itemDrag.pending||!route.empty()||!inWorld||!onlineScene||!latestState.viewport||moving||casting||npcOpen||npcWaiting||self.systemMenu.superview||[self.worldViewer pathPending])return;
 auto it=latestState.viewport->entities.find(pendingCombatTarget);if(it==latestState.viewport->entities.end()||it->second.dead){hasCombatTarget=NO;return;}auto& target=it->second;auto tile=[self.worldViewer movementTile];unsigned range=pendingCombatSkill?6:onlineScene->assets->characterResources.baseClass==2?6:3;
 unsigned approach=pendingCombatSkill?5:onlineScene->assets->characterResources.baseClass==2?5:1;auto& terrain=onlineScene->assets->terrain.attributes;if(terrain[target.ty*256+target.tx]&1){hasCombatTarget=NO;return;}if(std::hypot(double(tile.first)-target.tx,double(tile.second)-target.ty)>approach||(terrain[tile.second*256+tile.first]&1)){auto path=mu::viewer::find_path(onlineScene->assets->terrain.attributes,tile.first,tile.second,target.tx,target.ty);if(path.empty()){hasCombatTarget=NO;return;}constexpr int dx[]={-1,0,1,1,1,0,-1,-1},dy[]={-1,-1,-1,0,1,1,1,0};unsigned x=tile.first,y=tile.second;route.clear();for(unsigned d:path){route.push_back(d);x+=dx[d];y+=dy[d];if(route.size()>=3||(std::hypot(double(x)-target.tx,double(y)-target.ty)<=approach&&!(terrain[y*256+x]&1)))break;}routeRequested=NO;return;}
 if(NSProcessInfo.processInfo.systemUptime-lastCastSent<.6)return;if(combatAttempts>=5){hasCombatTarget=NO;[self.worldViewer setMessage:@"Server chưa xác nhận đòn đánh; chọn lại mục tiêu"];return;}[self attackMonster:pendingCombatTarget skill:pendingCombatSkill];
}
- (void)attackMonster:(unsigned)index skill:(BOOL)useSkill{
 if(casting||!inWorld||working||moving||npcOpen||npcWaiting||self.itemDrag.pending||!latestState.viewport)return;
 auto found=latestState.viewport->entities.find(index);if(found==latestState.viewport->entities.end()||found->second.kind!=mu::client::EntityKind::Monster||found->second.dead)return;
 unsigned skill=self.gameHud.activeSkill;if(useSkill&&skill!=4&&skill!=17){hasCombatTarget=NO;[self.worldViewer setMessage:@"Chọn Energy Ball hoặc Fire Ball trên thanh skill để test"];return;}
 auto tile=[self.worldViewer movementTile];auto& target=found->second;if((onlineScene->assets->terrain.attributes[tile.second*256+tile.first]&1)||(onlineScene->assets->terrain.attributes[target.ty*256+target.tx]&1)){[self.worldViewer setMessage:@"Ra khỏi vùng an toàn để đánh quái"];return;}
 unsigned range=useSkill?6:((onlineScene->assets->characterResources.baseClass==2)?6:3);if(std::hypot(double(tile.first)-target.tx,double(tile.second)-target.ty)>range){[self.worldViewer setMessage:@"Đi tới gần quái (trong 6 ô) rồi click để dùng skill"];return;}double now=[NSProcessInfo processInfo].systemUptime;if(now-lastCastSent<.35)return;lastCastSent=now;casting=YES;combatAttempts++;auto damageBefore=latestState.combat.damageRevision,castBefore=latestState.combat.castRevision;route.clear();routeRequested=NO;unsigned direction=(int(std::round(std::atan2(double(target.ty)-tile.second,double(target.tx)-tile.first)*4/M_PI))+11)%8;[self.worldViewer faceDirection:direction];
 dispatch_async(networkQueue,^{mu::client::WorldState next;std::string error;try{if(!session)throw std::runtime_error("Session disconnected");if(useSkill)session->castSkill(skill,index);else{session->attack(index,direction);dispatch_async(dispatch_get_main_queue(),^{[self.worldViewer playSpell:0];});}next=session->pump();}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{casting=NO;if(!error.empty()){hasCombatTarget=NO;[self.worldViewer setMessage:text(error)];return;}bool confirmed=useSkill&&next.combat.castRevision>castBefore&&next.combat.castTarget==index;for(const auto& hit:next.combat.hits)if(hit.revision>damageBefore&&hit.target==index)confirmed=true;if(confirmed&&pendingCombatTarget==index&&pendingCombatSkill==useSkill)hasCombatTarget=NO;[self applyCombat:next];latestState=next;[self.gameHud updateState:next];[self.characterInfo updateState:next];[self.worldViewer updateViewport:next.viewport];});});
}
- (void)buyShopItem:(unsigned)slot{[self shopAction:0 slot:slot];}
- (void)shopAction:(unsigned)action slot:(unsigned)slot{
 if(!inWorld||self.itemDrag.pending||self.warehousePanel.buying)return;
 self.warehousePanel.buying=YES;self.itemDrag.pending=YES;
 dispatch_async(networkQueue,^{mu::client::WorldState next;std::string error;bool accepted=false;std::shared_ptr<const mu::viewer::CharacterBody> body;try{if(!session)throw std::runtime_error("Session disconnected");auto before=session->state();if(action==0){unsigned destination=session->buyItem(slot);next=session->state();accepted=destination<236||next.money<before.money||(next.inventory&&before.inventory&&next.inventory->revision!=before.inventory->revision);}else{accepted=action==1?session->sellItem(slot):action==2?session->repairItem(slot):session->useBook(slot);next=session->state();}if(next.inventory){mu::ui::prepare_item_icons(profile.assetDirectory,profile.language,next.inventory->items);iconInventoryRevision=next.inventory->revision;}body=changedEquipment(profile,activeCharacter,next,equipmentKey);}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{self.itemDrag.pending=NO;self.warehousePanel.buying=NO;[self.itemDrag cancel];if(!error.empty()){[self.worldViewer setMessage:text(error)];return;}latestState=next;[self.inventoryPanel updateState:next];[self.warehousePanel updateState:next.npc];[self.characterInfo updateState:next];[self.itemDrag setAttributes:next.attributes];if(body)[self.worldViewer updateEquipment:body];[self.worldViewer setMessage:accepted?(action==0?@"Server đã xác nhận mua hàng":action==1?@"Server đã xác nhận bán đồ":action==2?@"Server đã cập nhật độ bền và Zen":@"Server đã xác nhận học kỹ năng"):@"Không có thay đổi từ server; kiểm tra Zen, chỗ trống và điều kiện item"];} );});
}
- (void)checkNpcReply{
 if(!npcWaiting)return;auto npc=latestState.npc;
 if(!npc.replied){if([NSProcessInfo processInfo].systemUptime-npcStarted<5)return;npcWaiting=NO;[self.worldViewer setMessage:@"NPC chưa trả lời; kiểm tra điều kiện NPC và log GameServer/DataServer"];dispatch_async(networkQueue,^{if(session)try{session->closeTalk();}catch(...){};});return;}
 if((npc.result==0||npc.result==2)&&!npc.itemsReady&&[NSProcessInfo processInfo].systemUptime-npcStarted<5)return;
 npcWaiting=NO;npcOpen=YES;if((npc.result==2||npc.result==0)&&npc.itemsReady){try{self.warehousePanel=[[MuWarehousePanel alloc] initWithState:npc assetRoot:profile.assetDirectory language:profile.language];self.warehousePanel.dragController=self.itemDrag;__weak ClientController* buyer=self;self.warehousePanel.onBuy=^(unsigned slot){[buyer buyShopItem:slot];};}catch(const std::exception& e){npcOpen=NO;[self.worldViewer setMessage:text(e.what())];dispatch_async(networkQueue,^{if(session)try{session->closeTalk();}catch(...){};});return;}__weak ClientController* owner=self;self.warehousePanel.onClose=^{ClientController* strong=owner;if(!strong)return;strong.itemDrag.sellView=nil;strong.itemDrag.onSell=nil;strong.inventoryPanel.repairMode=NO;strong.inventoryPanel.onRepair=nil;[strong.inventoryPanel close];[strong.worldViewer setInterfaceColumns:0];dispatch_async(strong->networkQueue,^{std::string error;try{if(strong->session)strong->session->closeTalk();}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{[strong.inventoryPanel close];strong->npcOpen=NO;[strong.worldViewer setMessage:error.empty()?@"Đã đóng Warehouse":text(error)];});});};try{if(!self.inventoryPanel){self.inventoryPanel=[[MuInventoryPanel alloc] initWithRoot:profile.assetDirectory language:profile.language];self.inventoryPanel.dragController=self.itemDrag;__weak ClientController* pairOwner=self;self.inventoryPanel.onClose=^{ClientController* strong=pairOwner;if(strong.warehousePanel.view.superview)[strong.warehousePanel close];else [strong.worldViewer setInterfaceColumns:0];};}__weak ClientController* bookOwner=self;self.inventoryPanel.onUseBook=^(unsigned slot){[bookOwner shopAction:3 slot:slot];};[self.inventoryPanel updateState:latestState];[self.inventoryPanel attachToGameView:self.worldViewer.interfaceView];[self.worldViewer setInterfaceColumns:1];}catch(const std::exception& e){[self.warehousePanel close];[self.worldViewer setMessage:text(e.what())];return;}if(npc.result==0){__weak ClientController* merchant=self;self.itemDrag.sellView=self.warehousePanel.view;self.itemDrag.onSell=^(unsigned slot){[merchant shopAction:1 slot:slot];};self.warehousePanel.onRepairMode=^(BOOL enabled){[merchant.itemDrag cancel];merchant.inventoryPanel.repairMode=enabled;};self.warehousePanel.onRepairAll=^{[merchant shopAction:2 slot:255];};self.inventoryPanel.onRepair=^(unsigned slot){[merchant shopAction:2 slot:slot];};}[self.warehousePanel attachToGameView:self.worldViewer.interfaceView rightOffset:190];[self.worldViewer setInterfaceColumns:2];[self.worldViewer setMessage:npc.result==0?@"Shop · click item để mua qua server":@"Warehouse · click nhấc/đặt giữa túi và kho · Esc để trả item"];return;}NSAlert* alert=[[NSAlert alloc] init];alert.messageText=npc.result==2?@"Warehouse":npc.result==0?@"Shop":npc.result==0xF9?@"Gens NPC":@"NPC";
 alert.informativeText=npc.result==0xF9?[NSString stringWithFormat:@"Server mở hội thoại NPC %u. Contribution: %u. Chưa có giao diện nhiệm vụ/Gens đầy đủ.",npc.npcClass,npc.contribution]:npc.itemsReady?[NSString stringWithFormat:@"Server đã mở giao diện, trả về %u vật phẩm thật. Chưa triển khai kéo/thả, mua bán và hình vật phẩm.",npc.itemCount]:@"Server đã phản hồi. Chưa có giao diện đầy đủ cho loại hội thoại này.";
 [alert addButtonWithTitle:@"Đóng hội thoại"];[alert beginSheetModalForWindow:self.worldViewer.window completionHandler:^(NSModalResponse response){dispatch_async(networkQueue,^{std::string error;try{if(session)session->closeTalk();}catch(const std::exception& e){error=e.what();}dispatch_async(dispatch_get_main_queue(),^{npcOpen=NO;[self.worldViewer setMessage:error.empty()?@"Đã đóng hội thoại NPC":text(error)];});});}];
}
- (void)keyboardMoveDirection:(unsigned)direction{
 if(self.systemMenu.superview)return;hasCombatTarget=NO;route.clear();routeRequested=NO;[self moveDirection:direction];}
- (void)destinationX:(unsigned)x y:(unsigned)y{
 if(self.systemMenu.superview)return;if(!authenticated||!inWorld)return;hasPendingLoot=NO;hasCombatTarget=NO;destinationX=x;destinationY=y;route.clear();routeRequested=YES;[self.worldViewer setMessage:[NSString stringWithFormat:@"Đi tới X=%u Y=%u",x,y]];}
- (void)prepareRoute{routeRequested=NO;try{auto start=[self.worldViewer movementTile];auto directions=mu::viewer::find_path(onlineScene->assets->terrain.attributes,start.first,start.second,destinationX,destinationY);route.assign(directions.begin(),directions.end());if(route.empty())[self.worldViewer setMessage:latestState.x==destinationX&&latestState.y==destinationY?@"Đã tới ô chọn":@"Không có đường tới ô chọn"];}catch(const std::exception& e){route.clear();[self.worldViewer setMessage:text(e.what())];}}
- (void)moveDirection:(unsigned)direction{
 if(self.itemDrag.pending||!authenticated||!inWorld||working||npcWaiting||npcOpen||!onlineScene)return;[self.worldViewer faceDirection:direction];if(moving)return;double interval=1./(([self.worldViewer isRunning]?3.75:3.)*onlineScene->movementRate);double now=[NSProcessInfo processInfo].systemUptime;if(now-lastInputTime<interval||now-lastActualInputTime<.04)return;lastInputTime=now-lastInputTime>.8?now:lastInputTime+interval;lastActualInputTime=now;
 constexpr int dx[8]={-1,0,1,1,1,0,-1,-1},dy[8]={-1,-1,-1,0,1,1,1,0};if(direction>=8)return;
 auto start=(!route.empty()&&[self.worldViewer pathPending]&&[self.worldViewer heldDirection]<0)?[self.worldViewer movementEndpoint]:[self.worldViewer movementTile];int x=int(start.first)+dx[direction],y=int(start.second)+dy[direction];
 if(x<0||y<0||x>=256||y>=256||(onlineScene->assets->terrain.attributes[y*256+x]&12)!=0){route.clear();routeRequested=NO;[self.worldViewer setMessage:@"Không thể đi vào ô bị chặn"];return;}
 BOOL routed=!route.empty()&&route.front()==direction&&[self.worldViewer heldDirection]<0;std::vector<unsigned> steps;if(routed){unsigned count=std::min<std::size_t>(14,route.size());steps.assign(route.begin(),route.begin()+count);route.erase(route.begin(),route.begin()+count);}else steps={direction};moving=YES;
 dispatch_async(networkQueue,^{mu::client::WorldState state;std::string error;
  try{if(!session)throw std::runtime_error("Session disconnected");state=session->movePath(start.first,start.second,steps);}catch(const std::exception& e){error=e.what();if(session)state=session->state();}
  dispatch_async(dispatch_get_main_queue(),^{moving=NO;if(error.empty()){[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];[self.worldViewer updateViewport:state.viewport];[self.worldViewer confirmedPathX:start.first y:start.second directions:steps];[self.worldViewer setMessage:[NSString stringWithFormat:@"Online · server xác nhận X=%u Y=%u · click để đi hoặc dùng mũi tên",state.x,state.y]];}else{route.clear();routeRequested=NO;if(error.rfind("Server",0)==0){[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];[self.worldViewer correctX:state.x y:state.y direction:state.direction];}[self.worldViewer setMessage:text(error)];}});
 });
}
- (void)toggleSystemMenu:(id)sender{
 if(!self.createPanel||working||self.itemDrag.pending)return;
 if(self.systemMenu.superview){[self.systemMenu close];return;}
 if(!self.systemMenu){try{self.systemMenu=[[MuSystemMenu alloc] initWithRoot:profile.assetDirectory lobby:self.lobby];__weak ClientController* weak=self;self.systemMenu.onAction=^(unsigned action){[weak performSystemAction:action];};}catch(const std::exception& e){[self append:text(e.what())];return;}}
 route.clear();routeRequested=NO;[self.worldViewer stopInput];
 try{[self.systemMenu presentIn:self.window.contentView world:inWorld authenticated:authenticated];}catch(const std::exception& e){[self append:text(e.what())];}
}
- (void)performSystemAction:(unsigned)action{
 if(working||self.itemDrag.pending||action>2)return;
 if(!authenticated){if(action==0)[NSApp terminate:nil];else if(action==1)[self logout:nil];return;}
 [self.itemDrag cancel];[self.warehousePanel close];[self.inventoryPanel close];route.clear();routeRequested=NO;[self.worldViewer stopInput];[self busy:YES];
 unsigned type=action==0?0:action==1?2:1;bool closeNpc=npcOpen;npcOpen=NO;npcWaiting=NO;
 dispatch_async(networkQueue,^{std::string error;auto list=std::make_shared<mu::client::CharacterList>();try{if(!session)throw std::runtime_error("Session disconnected");if(closeNpc)session->closeTalk();session->requestLogout(type);if(type==1)*list=session->characters();else session.reset();}catch(const std::exception& e){error=e.what();}
 dispatch_async(dispatch_get_main_queue(),^{if(!error.empty()){[self append:text(error)];[self.worldViewer setMessage:text(error)];[self busy:NO];return;}if(type==0){[NSApp terminate:nil];return;}inWorld=NO;authenticated=type==1;gameConnected=authenticated;characterList=*list;[self returnToLobby];[self.characters reloadData];[self refreshCharacterVisuals];[self busy:NO];});});
}
- (void)logout:(id)sender{
 self.createPanel.hidden=YES;self.characterMessage.hidden=YES;
 if(self.itemDrag.pending){self.logoutAfterTransfer=YES;return;}
 if(working||(!authenticated&&!inWorld&&!gameConnected))return;
 [self.itemDrag cancel];self.inventoryPanel.onClose=nil;self.warehousePanel.onClose=nil;[self.inventoryPanel close];[self.warehousePanel close];self.inventoryPanel=nil;self.warehousePanel=nil;[self busy:YES];route.clear();routeRequested=NO;npcWaiting=NO;npcOpen=NO;self.worldViewer.onEntity=nil;self.worldViewer.onDestination=nil;self.worldViewer.onMove=nil;self.worldViewer.onClose=nil;
 dispatch_async(networkQueue,^{session.reset();dispatch_async(dispatch_get_main_queue(),^{authenticated=NO;gameConnected=NO;inWorld=NO;onlineScene.reset();characterList={};[self returnToLobby];[self.characters reloadData];[self refreshCharacterVisuals];self.status.stringValue=@"";[self.worldViewer setMessage:@"Offline · đăng nhập lại để tiếp tục"];[self busy:NO];});});
}
- (void)keepalive:(NSTimer*)timer{
 if(inWorld&&self.window.isKeyWindow){groundNames.poll((NSEvent.modifierFlags&NSEventModifierFlagOption)!=0,CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState,49));self.gameHud.groundNamesLocked=groundNames.latched;}

 [self continueCombat];
 if(hasPendingLoot&&route.empty()&&!routeRequested&&![self.worldViewer pathPending]&&!moving)[self pickGround:pendingLoot];
 if(!self.systemMenu.superview&&authenticated&&inWorld&&!working&&!moving&&!npcWaiting&&!npcOpen){NSInteger direction=[self.worldViewer heldDirection];if(direction>=0)[self keyboardMoveDirection:unsigned(direction)];else{BOOL changed=routeRequested;if(changed)[self prepareRoute];if(!route.empty()&&(changed||![self.worldViewer pathPending]||[self.worldViewer pathNearEnd]))[self moveDirection:route.front()];}}
 if(!authenticated||working||polling||moving||self.itemDrag.pending)return;polling=YES;
 dispatch_async(networkQueue,^{mu::client::WorldState state;std::string error,equipmentError;std::shared_ptr<const mu::viewer::CharacterBody> refreshedBody;
  try{if(!session)throw std::runtime_error("Session disconnected");state=session->pump();if(state.inventory&&state.inventory->revision!=iconInventoryRevision){mu::ui::prepare_item_icons(profile.assetDirectory,profile.language,state.inventory->items);iconInventoryRevision=state.inventory->revision;}try{refreshedBody=changedEquipment(profile,activeCharacter,state,equipmentKey);}catch(const std::exception& e){equipmentError=e.what();}if(state.npc.items&&state.npc.items!=iconNpcItems){mu::ui::prepare_item_icons(profile.assetDirectory,profile.language,*state.npc.items);iconNpcItems=state.npc.items;}}catch(const std::exception& e){error=e.what();session.reset();}
  dispatch_async(dispatch_get_main_queue(),^{polling=NO;if(!error.empty()){npcWaiting=NO;npcOpen=NO;authenticated=NO;gameConnected=NO;inWorld=NO;[self returnToLobby];self.status.stringValue=@"Phiên kết nối đã đóng";[self append:text(error)];[self.worldViewer setMessage:text(error)];[self busy:NO];}else{if(!equipmentError.empty())[self append:text("Equipment resource: "+equipmentError)];[self applyCombat:state];latestState=state;[self.gameHud updateState:state];[self.characterInfo updateState:state];[self.itemDrag setAttributes:state.attributes];[self checkNpcReply];if(inWorld){if(refreshedBody)try{[self.worldViewer updateEquipment:refreshedBody];}catch(const std::exception& e){[self append:text(e.what())];}[self.inventoryPanel updateState:state];[self.warehousePanel updateState:state.npc];[self.worldViewer updateViewport:state.viewport];[self.worldViewer setBootsLevel:state.inventoryReady?state.bootsLevel:-1];[self.worldViewer updateX:state.x y:state.y direction:state.direction];}}});
 });
}
- (NSInteger)numberOfRowsInTableView:(NSTableView*)table{return table==self.table?servers.size():characterList.characters.size();}
- (id)tableView:(NSTableView*)table objectValueForTableColumn:(NSTableColumn*)column row:(NSInteger)row{
 if(row<0)return @"";
 if(table==self.table){if(std::size_t(row)>=servers.size())return @"";auto s=servers[row];if([column.identifier isEqual:@"Server"])return [NSString stringWithFormat:@"Server %u",s.code];if([column.identifier isEqual:@"Load"])return [NSString stringWithFormat:@"%u%%",s.load];return @"Khả dụng";}
 if(std::size_t(row)>=characterList.characters.size())return @"";const auto& c=characterList.characters[row];if([column.identifier isEqual:@"Nhân vật"])return text(c.name);if([column.identifier isEqual:@"Level"])return [NSString stringWithFormat:@"%u",c.level];if([column.identifier isEqual:@"Slot"])return [NSString stringWithFormat:@"%u",c.slot+1];return [NSString stringWithFormat:@"%u",c.control];
}
- (void)tableViewSelectionDidChange:(NSNotification*)notification{[self busy:working];}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app{return YES;}
- (void)applicationWillTerminate:(NSNotification*)notification{[self.timer invalidate];if(keyMonitor)[NSEvent removeMonitor:keyMonitor];}
@end
static void reportClientException(NSException* exception){NSString* report=[NSString stringWithFormat:@"%@\n%@\n%@\n",exception.name,exception.reason,[exception.callStackSymbols componentsJoinedByString:@"\n"]];[report writeToFile:@"/tmp/mu-client-exception.log" atomically:YES encoding:NSUTF8StringEncoding error:nil];}
int main(){@autoreleasepool{NSSetUncaughtExceptionHandler(reportClientException);NSApplication* app=[NSApplication sharedApplication];[app setActivationPolicy:NSApplicationActivationPolicyRegular];__attribute__((objc_precise_lifetime)) ClientController* controller=[[ClientController alloc] init];app.delegate=controller;[app run];}return 0;}
