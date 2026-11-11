import {itemRequirements} from './item-visual-policy.js';
import {requirementAttributeIndex} from './item-class-policy.js';
import {itemTooltipData as data} from './item-tooltip-data.js';
import {inventoryQuantity} from './item-quantity.js';
const colors=['#fff','#80b3ff','#ff3319','#ffcc19','#19ff80','#fff','#ff19ff','#fff','#fff','#fff','#666','#ff00ff','#b380ff','#ff9900'];
function format(template,values){let n=0,invalid=false;const text=template.replace(/%%|%[+\-0-9.]*[dsuf]/g,token=>{if(token==='%%')return '%';const value=values[n++];if(value===undefined){invalid=true;return '';}return String(value);});return invalid?undefined:text;}
export function itemDescriptionLines(item,meta,state,catalog){const row=data.items[item.index];if(!row)return [];const tierLevel=row.level+(item.index===6693?({1:1,2:2,4:3,8:4,16:5}[item.excellent]??0):(item.level??0)),levelRow=row.level>=0?data.levels[tierLevel]:undefined,refs=levelRow?.lines??(row.level<0?row.lines:[]),req=itemRequirements(item,meta),out=[];const actual=i=>i===5?state.vitals?.level??0:(state.attributes?.base?.[requirementAttributeIndex(i)]??0)+(state.attributes?.bonus?.[requirementAttributeIndex(i)]??0);
 for(const [id,color,bold]of refs){const spec=data.texts[id];if(!spec)continue;const status=spec.status;if(status===18||status===19||status>=105&&status<=200||status>=250&&status!==65535)continue;let values=[],bad=false;
 if(status===1)values=[item.durability];else if(status===5){values=[req[5]??0];bad=actual(5)<values[0];}else if(status>=6&&status<=10){const stat=[0,1,3,2,4][status-6];values=[req[stat]??0];bad=actual(stat)<values[0];}
 else if(status>=201&&status<=206){const stat=[5,0,1,3,2,4][status-201],diff=(req[stat]??0)-actual(stat);if(diff<=0)continue;values=[diff];bad=true;}
 else if(status===13){if(row.level<0&&(item.level??0)<5)continue;}
 else if(status===14||status===17)values=[(item.level??0)+1];else if(status===16)values=[item.durability*10];
 else if(status===215){const type=item.index&511,base=({30:7181,31:7182,136:7184,137:7190,138:7199,139:7209,140:7210,141:6159,142:7211,143:7212})[type];values=[inventoryQuantity(item),catalog[base]?.name];}
 else if(status===218)values=[actual(5)];else if(status===222){if(item.durability!==254)continue;}
 else if(status===223)values=[5-item.durability];else if(status>=207&&status<=214){const formulas={207:l=>l+32,208:l=>2*l+39,209:l=>2*l+20,210:l=>2*l+12,211:l=>2*l+25,212:l=>2*l+24,213:l=>2*l+10,214:l=>l+10};values=[formulas[status](item.level??0)];}
 else if(status!==65535)continue;
 const text=format(spec.text,values);if(text!==undefined)for(const line of text.split(/#|\r?\n/))if(line.trim())out.push({requirement:status>=5&&status<=10||status>=201&&status<=206,text:line,color:bad?'#ff3319':colors[color]??'#fff',bold:!!bold});}
 return out;
}
