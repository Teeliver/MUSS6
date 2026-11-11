import test from 'node:test';import assert from 'node:assert/strict';
import {PARTY_LIST_ROW_HEIGHT,partyListRows,partyListHeight,partyListDragBounds} from '../client/src/ui/party-list-policy.js';
test('party mini list keeps source 24px rows and collapse only hides non-leader rows',()=>{
 assert.equal(PARTY_LIST_ROW_HEIGHT,24);
 const members=[{name:'A'},{name:'B'},{name:'C'}];
 assert.deepEqual(partyListRows(members,false),members);
 assert.deepEqual(partyListRows(members,true),members.slice(0,1));
 assert.equal(partyListHeight(3,false),72);assert.equal(partyListHeight(3,true),24);assert.equal(partyListHeight(0,false),0);
});
test('party mini list dragging stays inside the viewport like NewUIPartyListWindow::MoveWindows',()=>{
 const rect={left:100,top:50,right:177,bottom:122};
 assert.deepEqual(partyListDragBounds(rect,10,20,1100,850),{x:10,y:20});
 assert.deepEqual(partyListDragBounds(rect,-200,-200,1100,850),{x:-100,y:-50});
 assert.deepEqual(partyListDragBounds(rect,2000,2000,1100,850),{x:923,y:728});
});
