import {skillSupport,skillCoverage} from './skill-support.js';
const classes=['DW — Dark Wizard','DK — Dark Knight','Elf','MG — Magic Gladiator','DL — Dark Lord','Summoner','RF — Rage Fighter'];
export function skillRows(document){
 if(document?.schema!==1||!Array.isArray(document.skills)||document.skills.length>1024)throw new Error('Invalid preview skill catalog');
 const ids=new Set();return document.skills.map(row=>{
  if(!Number.isInteger(row.index)||row.index<0||row.index>65535||ids.has(row.index)||typeof row.name!=='string'||row.name.length>128||!Array.isArray(row.requireClass)||row.requireClass.length!==7||row.requireClass.some(n=>!Number.isInteger(n)||n<0||n>3))throw new Error('Invalid preview skill row');ids.add(row.index);return {...row,requireClass:[...row.requireClass]};
 });
}
export function skillsFor(rows,cls,tier){
 if(!Number.isInteger(cls)||cls<0||cls>6||!Number.isInteger(tier)||tier<0||tier>2)throw new Error('Invalid preview class/tier');
 return rows.filter(row=>row.requireClass[cls]!==0&&tier+1>=row.requireClass[cls]).sort((a,b)=>a.index-b.index);
}
export function selectionUrl(current,{cls,tier,skill,freeze=false},rows){
 const url=new URL(current),oldClass=Number(url.searchParams.get('class')??0),allowed=skillsFor(rows,cls,tier);
 if(skill!==null&&!allowed.some(row=>row.index===skill))throw new Error('Skill does not match class/tier');
 const q=url.searchParams;q.set('preview','models');q.set('class',String(cls));q.set('tier',String(tier));
 if(oldClass!==cls)for(const key of ['loadout','weapon','armor','outfit','safe'])q.delete(key);
 for(const key of ['skill','effect','effect-time'])q.delete(key);
 if(skill===null){q.set('motion','cycle');}
 else{const support=skillSupport(skill);q.set('skill',String(skill));q.set('motion',support.effect?'cast':'idle');if(support.effect){q.set('effect',support.effect);if(freeze)q.set('effect-time','0.12');}}
 return url;
}
export async function createPreviewMenu(){
 const panel=document.querySelector('#preview-menu');panel.hidden=false;document.body.classList.add('preview-menu-open');
 const message=panel.querySelector('output');
 try{
  const response=await fetch(new URL('./preview-assets/skill-menu.json',location.href),{cache:'no-store'});if(!response.ok)throw new Error('Build và stage:preview để nạp danh sách skill');
  const text=await response.text();if(text.length>262144)throw new Error('Preview skill catalog too large');const rows=skillRows(JSON.parse(text));
  const clsInput=panel.querySelector('[name="class"]'),tierInput=panel.querySelector('[name="tier"]'),skillInput=panel.querySelector('[name="skill"]'),freezeInput=panel.querySelector('[name="freeze"]');
  const query=new URL(location.href).searchParams;let cls=Number(query.get('class')??0),tier=Number(query.get('tier')??0);if(!Number.isInteger(cls)||cls<0||cls>6)cls=0;if(!Number.isInteger(tier)||tier<0||tier>2)tier=0;
  for(const [index,name] of classes.entries()){const option=document.createElement('option');option.value=String(index);option.textContent=name;clsInput.append(option);}clsInput.value=String(cls);tierInput.value=String(tier);
  const available=skillsFor(rows,cls,tier),requested=query.has('skill')?Number(query.get('skill')):query.get('effect')==='fire'?4:query.get('effect')==='energy'?17:query.get('effect')==='spirit'?9:null;
  const selected=available.some(row=>row.index===requested)?requested:null;
  const none=document.createElement('option');none.value='';none.textContent='Chỉ model / animation';skillInput.append(none);
  for(const row of available){const option=document.createElement('option');option.value=String(row.index);option.textContent=`${row.index} · ${row.name} — ${skillSupport(row.index).label}`;skillInput.append(option);}skillInput.value=selected===null?'':String(selected);
  freezeInput.checked=query.has('effect-time');freezeInput.disabled=!skillSupport(selected).effect;
  message.textContent=[4,9,17].includes(selected)?`${selected===9?'Evil Spirit':selected===4?'Fire01':'Energy'} thử: cast + projectile. Chưa hit/damage/server.`:selected===null?'Chọn skill để kiểm tra. Danh sách chỉ lọc class/cấp chuyển chức.':'Skill này chưa có effect riêng trong preview; đang hiển thị model, không dùng Fire01 thay thế.';
  const coverage=skillCoverage(available);panel.querySelector('[data-coverage]').textContent=`${coverage.total} skill trong config: ${coverage.prototype} effect thử, ${coverage.missing} chưa ghép. Chưa có skill nào được xác nhận hoàn chỉnh.`;
  panel.querySelectorAll('select').forEach(input=>input.disabled=false);
  const current=selectionUrl(location.href,{cls,tier,skill:selected,freeze:freezeInput.checked},rows);
  if(!query.has('skill')&&!query.has('effect')&&['idle','walk','run','cycle','combat','attack','cast'].includes(query.get('motion')))current.searchParams.set('motion',query.get('motion'));
  history.replaceState(null,'',current);
  const change=()=>{
   const nextClass=Number(clsInput.value),nextTier=Number(tierInput.value);let skill=skillInput.value===''?null:Number(skillInput.value);if(!skillsFor(rows,nextClass,nextTier).some(row=>row.index===skill))skill=null;
   location.assign(selectionUrl(location.href,{cls:nextClass,tier:nextTier,skill,freeze:freezeInput.checked},rows).href);
  };
  for(const input of [clsInput,tierInput,skillInput,freezeInput])input.addEventListener('change',change);
  return ()=>{for(const input of [clsInput,tierInput,skillInput,freezeInput])input.removeEventListener('change',change);};
 }catch(error){message.textContent=`Menu: ${error.message}`;return ()=>{};}
}
