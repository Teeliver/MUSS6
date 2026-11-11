import {loadMapCatalog} from './map-catalog.js';
export async function createMapMenu({document=globalThis.document,target=globalThis.window,getWorld=()=>undefined}={}){
 const catalog=await loadMapCatalog(),panel=document.createElement('aside');panel.id='map-menu';panel.hidden=true;panel.setAttribute('role','dialog');panel.setAttribute('aria-label','Move');
 const title=document.createElement('h2');title.textContent='Move';
 const restoreFocus=()=>{const canvas=document.querySelector('canvas');if(canvas){canvas.tabIndex=0;canvas.focus({preventScroll:true});}};
 const close=document.createElement('button');close.className='mu-move-close';close.textContent='×';close.setAttribute('aria-label','Đóng Move');close.addEventListener('click',()=>{panel.hidden=true;restoreFocus();});
 const result=document.createElement('output');result.setAttribute('role','status');
 const header=document.createElement('div');header.className='mu-move-head';for(const name of ['Map','Level','Zen']){const label=document.createElement('span');label.textContent=name;header.append(label);}
 const moves=document.createElement('div');moves.className='mu-move-list';const buttons=[];let busy=false;
 for(const row of catalog.moves){const record=catalog.worlds.find(w=>w.map===row.map),button=document.createElement('button');button.className='mu-move-row';button.setAttribute('aria-label',`${row.name} · Lv.${row.minLevel??0} · ${row.money} Zen`);
  for(const value of [row.name,String(row.minLevel??0),Number(row.money).toLocaleString('en-US')]){const label=document.createElement('span');label.textContent=value;button.append(label);}
  buttons.push({button,record,row});button.addEventListener('click',async()=>{const world=getWorld();if(!world?.position||busy)return;busy=true;refresh();result.textContent=`Đang chuyển ${row.name}…`;try{await world.travel(row.id);panel.hidden=true;restoreFocus();result.textContent='';}catch(e){result.textContent=e.message;}finally{busy=false;refresh();}});moves.append(button);
 }
 const worlds=document.createElement('details');worlds.className='mu-move-debug';const summary=document.createElement('summary');summary.textContent='Kiểm tra Data (offline)';worlds.append(summary);
 for(const row of catalog.worlds){const link=document.createElement('a');link.href=`?scene=terrain-check&map=${row.map}`;link.target='_blank';link.rel='noopener';link.textContent=`Map${row.map} / World${row.world} ${row.names.join(', ')}`;worlds.append(link);}
 const refresh=()=>{const world=getWorld(),state=world?.gameState;for(const {button,record,row} of buttons){const lowLevel=state?.vitals?.level!==undefined&&state.vitals.level<(row.minLevel??0),poor=state?.money!==undefined&&state.money<row.money;button.disabled=busy||!world?.position||record?.terrain!=='loadable'||lowLevel||poor;}worlds.hidden=!!world?.position;};
 panel.append(title,close,header,moves,result,worlds);document.body.append(panel);
 const launcher=document.createElement('button');launcher.id='map-launcher';launcher.textContent='Move (M)';const toggle=()=>{panel.hidden=!panel.hidden;if(!panel.hidden){refresh();close.focus();}else restoreFocus();};launcher.addEventListener('click',toggle);document.body.append(launcher);
 const key=event=>{if(event.ctrlKey||event.altKey||event.metaKey||event.repeat||event.target?.closest?.('input,textarea,select,[contenteditable]'))return;if(event.key.toLowerCase()==='m'){event.preventDefault();toggle();}else if(event.key==='Escape'&&!panel.hidden){event.preventDefault();panel.hidden=true;restoreFocus();}};target.addEventListener('keydown',key);
 return ()=>{target.removeEventListener('keydown',key);panel.remove();launcher.remove();};
}
