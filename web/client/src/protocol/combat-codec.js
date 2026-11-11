// ENG/UPDATE603/EXTRA1; native client normal_attack_packet, Protocol.h MSVC layout.
export function normalAttackPacket(target,direction){if(!Number.isInteger(target)||target<0||target>=0x4000||!Number.isInteger(direction)||direction<0||direction>7)throw new Error('Invalid attack');return Uint8Array.of(0xc1,7,0x11,target>>8,target&255,120,direction);}
export function combatEvent(bytes){
 if(!(bytes instanceof Uint8Array)||bytes.length<3||![0xc1,0xc3].includes(bytes[0])||![0x11,0x18,0x9c,0x17,0x26,0x19,0x1e].includes(bytes[2]))return null;
 if(bytes[1]!==bytes.length)throw new Error('Invalid combat frame');const d=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength),index=((bytes[3]&0x7f)<<8)|bytes[4];
 if(bytes[2]===0x19||bytes[2]===0x1e){const duration=bytes[2]===0x1e;if(bytes.length!==(duration?10:9))throw Error('Invalid skill broadcast');return {kind:'skill',skill:bytes[3]*256+bytes[4],index:bytes[5]*256+bytes[6],...(duration?{x:bytes[7],y:bytes[8],angle:bytes[9]}:{target:((bytes[7]&63)<<8)|bytes[8],success:!!(bytes[7]&128)})};}
 if(bytes[2]===0x17){if(bytes.length!==9)throw new Error('Invalid death');return {kind:'death',index};}
 if(bytes[2]===0x26){if(bytes.length!==20||![0xfd,0xfe,0xff].includes(bytes[3]))throw new Error('Invalid life');return {kind:'life',type:bytes[3],hp:d.getUint32(12,true),sd:d.getUint32(16,true)};}
 if(bytes[2]===0x18){if(bytes.length!==9||bytes[5]>15)throw new Error('Invalid action');return {kind:'action',index,direction:bytes[5],action:bytes[6],target:bytes[7]*256+bytes[8]};}
 if(bytes[2]===0x11){if(bytes.length!==28)throw new Error('Invalid damage');return {kind:'damage',index,damage:d.getUint32(20,true),shieldDamage:d.getUint32(24,true),hp:d.getUint32(12,true),sd:d.getUint32(16,true),type:bytes[7]};}
 if(bytes.length!==24)throw new Error('Invalid reward');return {kind:'kill',index,damage:d.getUint32(12,true),experience:d.getUint32(16,true),nextExperience:d.getUint32(20,true)};
}

export function regenEvent(b){if(b.length!==44||![0xc1,0xc3].includes(b[0])||b[1]!==44||b[2]!==0xf3||b[3]!==4||b[7]>7)throw new Error('Invalid regen');const d=new DataView(b.buffer,b.byteOffset,b.byteLength);return {x:b[4],y:b[5],map:b[6],direction:b[7],money:d.getUint32(24,true),hp:d.getUint32(28,true)};}
