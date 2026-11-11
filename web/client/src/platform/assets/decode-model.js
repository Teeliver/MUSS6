// Direct BMD v10/v12 bytes; this stage returns inspection metadata, not GPU buffers.
export function inspectBmdModel(core,bytes){
 if(!(bytes instanceof Uint8Array)||bytes.byteLength>16*1024*1024)throw new Error('Invalid BMD byte input');
 const result=core.bmdModelResult(bytes);if(!result.ok)throw new Error(result.error);
 const {ok,...model}=result;return model;
}

export function decodeBmdGeometry(core,bytes){
 if(!(bytes instanceof Uint8Array)||bytes.byteLength>16*1024*1024)throw new Error('Invalid BMD byte input');
 const result=core.bmdGeometryResult(bytes);if(!result.ok)throw new Error(result.error);
 return {meshes:result.meshes.map((mesh,sourceMeshOrdinal)=>({...mesh,sourceMeshOrdinal}))};
}

export function decodeBoundGeometry(core,equipment,skeleton,{action=1,frame=0}={}){
 if(![equipment,skeleton].every(b=>b instanceof Uint8Array&&b.length<=16*1024*1024)||!Number.isInteger(action)||action<0||action>=512||!Number.isInteger(frame)||frame<0||frame>=256)throw new Error('Invalid equipment/Player pose input');
 const result=core.boundGeometryResult(equipment,skeleton,action,frame);if(!result.ok)throw new Error(result.error);return {meshes:result.meshes.map((mesh,sourceMeshOrdinal)=>({...mesh,sourceMeshOrdinal}))};
}

export function decodeInventoryArmorGeometry(core,equipment,skeleton,index){
 if(![equipment,skeleton].every(b=>b instanceof Uint8Array&&b.length<=16*1024*1024)||![4155,4156,4157,4169,4179].includes(index))throw Error('Invalid native inventory armor');
 const result=core.inventoryArmorGeometryResult(equipment,skeleton,index);if(!result.ok)throw Error(result.error);return {meshes:result.meshes.map((mesh,sourceMeshOrdinal)=>({...mesh,sourceMeshOrdinal}))};
}

export function decodeAttachmentGeometry(core,item,player,{action=1,frame=0,slot=0,index=0,safe=false,rfSatan=false}={}){
 if(![item,player].every(b=>b instanceof Uint8Array&&b.length<=16*1024*1024)||!Number.isInteger(action)||action<0||action>=512||!Number.isInteger(frame)||frame<0||frame>=256||![0,1,7,8].includes(slot)||!Number.isInteger(index)||index<0||index>=8192||(typeof safe!=='boolean'||typeof rfSatan!=='boolean'))throw new Error('Invalid attachment geometry input');
 const result=core.attachmentGeometryResult(item,player,action,frame,slot,index,safe,rfSatan);if(!result.ok)throw new Error(result.error);return {meshes:result.meshes.map((mesh,sourceMeshOrdinal)=>({...mesh,sourceMeshOrdinal}))};
}
