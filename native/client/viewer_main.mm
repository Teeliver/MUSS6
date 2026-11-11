#include "world_view.hpp"
#include "client.hpp"
#include <iostream>
#include <cstring>
@interface ViewerDelegate:NSObject<NSApplicationDelegate>
@end
@implementation ViewerDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app{return YES;}
@end
int main(int argc,char** argv){@autoreleasepool{try{
 auto profile=mu::client::load_profile();unsigned x=135,y=124;
 if(argc!=1&&argc!=3)throw std::runtime_error("Usage: MuClient-viewer [X Y]");
 if(argc==3){std::size_t a=0,b=0;x=std::stoul(argv[1],&a);y=std::stoul(argv[2],&b);if(a!=std::strlen(argv[1])||b!=std::strlen(argv[2]))throw std::runtime_error("Invalid map coordinates");}
 auto data=mu::viewer::load(profile.assetDirectory,0,x,y,0);
 auto app=[NSApplication sharedApplication];__attribute__((objc_precise_lifetime)) ViewerDelegate* delegate=[[ViewerDelegate alloc] init];app.delegate=delegate;[app setActivationPolicy:NSApplicationActivationPolicyRegular];
 NSMenu* menuBar=[[NSMenu alloc] init];NSMenuItem* menuItem=[[NSMenuItem alloc] init];[menuBar addItem:menuItem];NSMenu* appMenu=[[NSMenu alloc] initWithTitle:@"MU World Viewer"];[appMenu addItemWithTitle:@"Quit MU World Viewer" action:@selector(terminate:) keyEquivalent:@"q"];menuItem.submenu=appMenu;app.mainMenu=menuBar;
 __attribute__((objc_precise_lifetime)) MuWorldViewer* viewer=[[MuWorldViewer alloc] init];[viewer showScene:data];[app run];return 0;
 }catch(const std::exception& e){std::cerr<<"World viewer: "<<e.what()<<"\n";return 1;}}}
