// F3:03 snapshot only: subsequent combat/stat updates are not integrated yet.
export function entrySummary(row,entry){
 return `${row.name} · Lv.${row.level} | Map ${entry.map} (${entry.x}, ${entry.y}) | HP ${entry.hp}/${entry.maxHp} | Zen ${entry.money}`;
}
export function createEntryHud(document){
 const panel=document.createElement('aside');panel.id='entry-hud';
 panel.setAttribute('aria-label','Thông tin khi vào map');panel.hidden=true;
 const values=document.createElement('output'),notice=document.createElement('small');
 notice.textContent='HP cập nhật theo server · Zen lúc vào map/hồi sinh';
 panel.append(values,notice);document.body.append(panel);
 return {show(row,entry){values.textContent=entrySummary(row,entry);panel.hidden=false;},clear(){values.textContent='';panel.hidden=true;},dispose(){panel.remove();}};
}
