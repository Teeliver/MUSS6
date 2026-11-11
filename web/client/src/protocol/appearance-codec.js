export function decodeAppearance(core,bytes){
 const result=core.appearanceResult(bytes);
 if(!result.ok)throw new Error(result.error);
 const {ok,...appearance}=result;return appearance;
}

export function decodeCompanions(core,bytes){
 const result=core.companionsResult(bytes);if(!result.ok)throw new Error(result.error);
 const {ok,...companions}=result;return companions;
}

// The inventory snapshot is authoritative after entry/equipment ACK; roster CharSet can be stale.
export function equippedAppearance(appearance,items){const equipped=new Map(items.filter(i=>i.slot<7).map(i=>[i.slot,i]));return {...appearance,parts:appearance.parts.map((part,slot)=>{const item=equipped.get(slot);return {...part,present:!!item,index:item?.index??part.index,visualLevel:item?.level??0,excellent:!!item?.excellent,ancient:!!(item?.setOption&3)};})};}
