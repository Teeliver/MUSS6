#include "system_menu.hpp"
#include "source_font.hpp"
#include <iostream>
#include <stdexcept>
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){@autoreleasepool{try{
 need(argc>=2,"Usage: system_menu_contract CLIENT_ROOT [SNAPSHOT_PREFIX]");[NSApplication sharedApplication];std::string root=argv[1];mu::ui::prepare_source_font(root,"Eng");auto lobby=[[MuLobbyView alloc] initWithRoot:root language:"Eng"];
 auto window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,1100,825) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];auto parent=[[NSView alloc] initWithFrame:NSMakeRect(0,0,1100,825)];window.contentView=parent;auto menu=[[MuSystemMenu alloc] initWithRoot:root lobby:lobby];__block unsigned action=99;menu.onAction=^(unsigned a){action=a;};
 for(unsigned stage=0;stage<3;stage++){
  [menu presentIn:parent world:stage==2 authenticated:stage!=0];need(menu.superview==parent&&NSEqualRects(menu.frame,parent.bounds),"Menu must remain inside game viewport");auto content=menu.subviews.lastObject;need(NSContainsRect(menu.bounds,content.frame),"Menu frame escaped viewport");unsigned count=0;NSButton* cancel=nil;NSButton* server=nil;for(NSView* view in content.subviews)if([view isKindOfClass:NSButton.class]){auto button=(NSButton*)view;count++;need(NSContainsRect(content.bounds,button.frame),"Menu button escaped panel");need(button.title.length>0,"Menu label missing from language data");if(button.tag==4)cancel=button;if(button.tag==1)server=button;}
  need(count==(stage==2?5:stage==1?4:3),"Wrong menu for scene");need(cancel!=nil,"Missing cancel");
  if(argc==3){auto bitmap=[menu bitmapImageRepForCachingDisplayInRect:menu.bounds];[menu cacheDisplayInRect:menu.bounds toBitmapImageRep:bitmap];NSData* png=[bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];need(png.length>0,"Menu snapshot missing");[png writeToFile:[NSString stringWithFormat:@"%s-%u.png",argv[2],stage] atomically:YES];}
  [cancel performClick:nil];need(!menu.superview&&action==99,"Cancel must dismiss without logout");
  if(stage){[menu presentIn:parent world:stage==2 authenticated:YES];content=menu.subviews.lastObject;for(NSView* view in content.subviews)if([view isKindOfClass:NSButton.class]&&((NSButton*)view).tag==1)server=(NSButton*)view;[server performClick:nil];need(action==1&&!menu.superview,"Server selection callback failed");action=99;}
 }
 std::cout<<"PASS: login/character/world menus, Data labels, modal bounds, cancel and server callback.\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}}
