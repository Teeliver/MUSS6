#pragma once
namespace mu::input {
struct GroundNames {
 bool optionHeld=false,latched=false,spaceHeld=false;
 void flags(bool option){if(option&&!optionHeld)latched=false;optionHeld=option;}
 void poll(bool option,bool space){flags(option);if(option&&space&&!spaceHeld)lock();spaceHeld=space;}
 void lock(){optionHeld=true;latched=true;}
};
}
