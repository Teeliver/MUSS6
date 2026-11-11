import {itemModelBindings} from './item-model-bindings.js';
export function itemResource(core,index,level){
 if(!Number.isInteger(index)||index<0||index>=8192||!Number.isInteger(level)||level<0||level>15)throw new Error('Invalid item resource identity');
 const result=core.itemResourceResult(index,level);if(!result.ok)throw new Error(result.error);
 return result.mapped?{mapped:true,path:result.path}:itemModelBindings[index]?{mapped:true,path:itemModelBindings[index]}:{mapped:false,path:''};
}
