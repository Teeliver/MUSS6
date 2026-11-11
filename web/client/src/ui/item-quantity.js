// CNewUIInventoryCtrl::RenderNumberOfItem and COMGEM::Check_Jewel_Com.
// Wire level has already been decoded (do not shift it a second time).
export function inventoryQuantity(item){const group=item.index>>9,type=item.index&511;
 if(group===12&&([30,31].includes(type)||type>=136&&type<=143))return ((item.level??0)+1)*10;
 const native=group===14&&(type<=8||type>=35&&type<=40||type>=46&&type<=50||[70,71,94,133].includes(type)||type>=78&&type<=82||type>=85&&type<=90);
 // Current GameServer Data/Item/ItemStack.txt: extra stackable items. These
 // are quantities, unlike the durability on weapons, armor and pets.
 const configured=group===4&&[7,15].includes(type)||group===13&&[32,33,34].includes(type)||group===14&&(type===21&&item.level===3||type===100);
 return (native||configured)&&item.durability>1?item.durability:undefined;
}
// Regular text avoids the heavy bitmap's bright fringes at browser/DPR scaling.
export function paintInventoryQuantity(ctx,atlas,value,width){ctx.font='600 9px Arial';ctx.textAlign='right';ctx.textBaseline='top';ctx.fillStyle='#dedede';ctx.shadowColor='#000';ctx.shadowOffsetX=1;ctx.shadowOffsetY=1;ctx.shadowBlur=0;ctx.fillText(String(value),width-2,1);ctx.shadowOffsetX=ctx.shadowOffsetY=0;}
