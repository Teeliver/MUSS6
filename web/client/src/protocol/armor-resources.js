// RenderPartObject body resources differ from item icon resources.
export function armorResource(core,part,type,baseClass,tier){
 for(const [value,max] of [[part,4],[type,511],[baseClass,6],[tier,2]])if(!Number.isInteger(value)||value<0||value>max)throw new Error('Invalid armor resource identity');
 const result=core.armorResourceResult(part,type,baseClass,tier);
 if(!result.ok)throw new Error(result.error);return result.path;
}
