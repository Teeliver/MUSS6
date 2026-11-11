// NewUIPartyListWindow.cpp: PARTY_LIST_WINDOW_HEIGHT/m_iVal are 24 px.
// Collapse only hides rows after the leader; drag keeps the window on screen.
export const PARTY_LIST_ROW_HEIGHT=24;
export function partyListRows(members,collapsed){const list=members??[];return collapsed?list.slice(0,1):list;}
export function partyListHeight(count,collapsed){const rows=Math.max(0,Number(count)||0);return (collapsed?Math.min(1,rows):rows)*PARTY_LIST_ROW_HEIGHT;}
export function partyListDragBounds(rect,dx,dy,viewportWidth,viewportHeight){const clamp=(value,min,max)=>Math.max(min,Math.min(max,value));return {x:clamp(dx,-rect.left,viewportWidth-rect.right),y:clamp(dy,-rect.top,viewportHeight-rect.bottom)};}
