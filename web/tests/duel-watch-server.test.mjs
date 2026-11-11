import test from 'node:test';import assert from 'node:assert/strict';import {readFile,mkdtemp,writeFile,rm} from 'node:fs/promises';import {execFileSync} from 'node:child_process';import {tmpdir} from 'node:os';import path from 'node:path';
test('actual Duel leave handler retains membership and buffs when gate fails; commits only successful exit',async()=>{const source=await readFile(new URL('../../GameServer/Duel.cpp',import.meta.url),'utf8'),start=source.indexOf('void CDuel::CGDuelButtonRecv('),end=source.indexOf('void CDuel::GCDuelStartSend(',start);assert.ok(start>0&&end>start);const dir=await mkdtemp(path.join(tmpdir(),'mu-duel-leave-contract-'));try{const cpp=path.join(dir,'contract.cpp'),bin=path.join(dir,'contract');await writeFile(cpp,`#include <vector>
#include <cassert>
#define GAMESERVER_UPDATE 603
#define MAX_DUEL_ARENA 4
#define CHECK_RANGE(n,m) ((n)>=0&&(n)<(m))
#define EFFECT_INVISIBILITY 1
#define EFFECT_DUEL_ARENA_WATCH 2
struct OBJECT{}; using LPOBJ=OBJECT*; OBJECT gObj[1];
struct PMSG_DUEL_BUTTON_RECV{int number;};struct DUEL_ARENA_INFO{};
bool connected=true,member=true,gateWorks=true;std::vector<int> calls;
int gObjIsConnectedGP(int){return connected;}int gObjMoveGate(int,int){calls.push_back(1);return gateWorks;}
struct Effects{void DelEffect(LPOBJ,int id){calls.push_back(1+id);}}gEffectManager;
struct CDuel{DUEL_ARENA_INFO m_DuelArenaInfo[4];int GetDuelArenaSpectator(DUEL_ARENA_INFO*,int){return member;}void GCDuelSpectatorDelSend(int,DUEL_ARENA_INFO*){calls.push_back(4);}void DelDuelArenaSpectator(DUEL_ARENA_INFO*,int){calls.push_back(5);member=false;}void CGDuelButtonRecv(PMSG_DUEL_BUTTON_RECV*,int);};
${source.slice(start,end)}
int main(){CDuel duel;PMSG_DUEL_BUTTON_RECV msg{0};gateWorks=false;duel.CGDuelButtonRecv(&msg,0);assert((calls==std::vector<int>{1}));assert(member);calls.clear();gateWorks=true;duel.CGDuelButtonRecv(&msg,0);assert((calls==std::vector<int>{1,2,3,4,5}));assert(!member);calls.clear();duel.CGDuelButtonRecv(&msg,0);assert(calls.empty());member=true;msg.number=4;duel.CGDuelButtonRecv(&msg,0);assert(calls.empty());msg.number=0;connected=false;duel.CGDuelButtonRecv(&msg,0);assert(calls.empty());}
`);execFileSync('clang++',['-std=c++17',cpp,'-o',bin]);execFileSync(bin);}finally{await rm(dir,{recursive:true,force:true});}});
