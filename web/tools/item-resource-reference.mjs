export function itemResourceReference(source){
 const start=source.indexOf('namespace mu::viewer');if(start<0)throw new Error('Native item mapping reference missing');
 return '#pragma once\n#include <filesystem>\n#include <string>\n#include <cstdio>\nnamespace mu::client{struct Item{unsigned index,level;};}\n'+source.slice(start);
}
