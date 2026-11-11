#pragma once
#import <Cocoa/Cocoa.h>
#include <string>
namespace mu::ui {
void prepare_source_font(const std::string& root,const std::string& language);
NSFont* source_font(CGFloat size,bool bold=false);
}
