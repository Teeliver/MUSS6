export function equipmentGrip(core,slot,index,safe){
 if(![0,1,7].includes(slot)||!Number.isInteger(index)||index<0||index>=8192||typeof safe!=='boolean')throw new Error('Invalid equipment attachment identity');
 const result=core.equipmentGripResult(slot,index,safe);if(!result.ok)throw new Error(result.error);
 return Object.freeze({bone:result.bone,transform:Object.freeze([...result.transform])});
}
