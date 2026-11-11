"""Source-backed constant CreateCharacter monster bindings and all spawn sections."""
import argparse,ast,hashlib,json,re,shlex
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--server',type=Path,required=True);p.add_argument('--catalog',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
def clean(s):return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]) if not m[0].startswith('"') else m[0],s)
def number(expr):
 expr=re.sub(r'(?<=[\d.])[fF]\b','',expr).strip();node=ast.parse(expr,mode='eval').body
 def n(x):
  if isinstance(x,ast.Constant) and isinstance(x.value,(int,float)):return x.value
  if isinstance(x,ast.UnaryOp) and isinstance(x.op,ast.USub):return -n(x.operand)
  if isinstance(x,ast.BinOp) and isinstance(x.op,(ast.Add,ast.Sub,ast.Mult,ast.Div)):return {ast.Add:lambda a,b:a+b,ast.Sub:lambda a,b:a-b,ast.Mult:lambda a,b:a*b,ast.Div:lambda a,b:a/b}[type(x.op)](n(x.left),n(x.right))
  raise ValueError('dynamic expression')
 return n(node)
def rows(file):
 for i,line in enumerate(file.read_text('latin1').splitlines(),1):
  s=line.split('//')[0].strip()
  if s:yield i,shlex.split(s)
monsters={int(r[0]):{'type':int(r[0]),'npc':int(r[1])==1,'name':r[2]} for _,r in rows(a.server/'Monster/MonsterList.txt') if r[0]!='end'}
catalog=json.loads(a.catalog.read_text());actors={k:v for k,v in catalog['actors'].items() if not v.get('policy','').startswith('Explicit constant CreateCharacter')};catalog['actors']=actors;found={};conflicts=[]
for file in sorted(a.source.glob('*.cpp')):
 text=file.read_text('latin1');s=clean(text)
 for fn in re.finditer(r'CHARACTER\s*\*\s*[\w:]*Create[\w]*(?:Monster|Characters)\s*\([^)]*\)\s*\{',s):
  start=fn.end();depth=1;end=start
  while end<len(s) and depth:depth+=(s[end]=='{')-(s[end]=='}');end+=1
  body=s[start:end-1]
  # Cases at the main switch level; never consume nested state/action switches.
  depth=0;atDepth={};pos=0
  for token in re.finditer(r'[{}]|\bcase\s+(\d+)\s*:',body):
   if token[0]=='{':depth+=1
   elif token[0]=='}':depth-=1
   elif token[1]:atDepth.setdefault(depth,[]).append((token.start(),token.end(),int(token[1])))
  if not atDepth:continue
  level=min(atDepth);cases=atDepth[level]
  for i,(begin,after,t) in enumerate(cases):
   if t not in monsters or monsters[t]['npc']:continue
   stop=cases[i+1][0] if i+1<len(cases) else len(body);block=body[after:stop]
   j=i+1
   while not block.strip() and j<len(cases):
    stop=cases[j+1][0] if j+1<len(cases) else len(body);block=body[cases[j][1]:stop];j+=1
   models=re.findall(r'CreateCharacter\(\s*[A-Za-z_]\w*\s*,\s*MODEL_MONSTER01\s*(\+\s*[^,]+)?\s*,',block)
   try:indices={int(number(x.lstrip('+'))) if x else 0 for x in models}
   except (ValueError,SyntaxError,TypeError):continue
   if len(indices)!=1:continue
   index=indices.pop();scales=re.findall(r'(?:\w+->Object\.|o->)Scale\s*=\s*([^;]+);',block)
   try:values={float(number(x)) for x in scales}
   except (ValueError,SyntaxError,TypeError):continue
   variants={0:(.8,0),4:(1.15,-1),8:(1,-1),1:(.85,0),5:(1.1,1),6:(.85,-1),9:(1.1,-1),58:(1.8,-1),59:(2.1,-1),61:(1.5,-1),63:(1.9,-1),71:(1.1,-1),74:(1.3,-1),301:(1.3,-1),73:(.8,-1),75:(1,-1)}
   if len(values)>1 and t not in variants:continue
   scale,hidden=variants.get(t,(next(iter(values)) if values else .9,-1))
   hiddenMatches=re.findall(r'(?:\w+->Object\.|o->)HiddenMesh\s*=\s*(-?\d+)',block)
   if t not in variants and len(set(hiddenMatches))==1:hidden=int(hiddenMatches[0])
   weapons=re.findall(r'Weapon\[(\d)\]\.Type\s*=\s*(MODEL_\w+\s*\+\s*\d+)',block)
   row={**monsters[t],'kind':'monster','files':[f'Monster/Monster{index+1:02}.bmd'],'scale':scale,'opacity':float(re.search(r'(?:Object\.|o->)AlphaTarget\s*=\s*([\d.]+)',block)[1]) if re.search(r'(?:Object\.|o->)AlphaTarget\s*=\s*([\d.]+)',block) else 1,'altitude':0,'additive':-1,'hiddenMesh':hidden,'hidden':hidden==-2,'idleKeys':6.25,'walkKeys':8.5,'attackKeys':8.25,'deathKeys':13.75,'monsterModel':index,'weaponsSource':weapons,'bindingSource':{'file':file.name,'line':text.count('\n',0,start+begin)+1,'sha256':hashlib.sha256(file.read_bytes()).hexdigest()},'renderWarnings':(['Monster weapon attachments need source-specific bone routing'] if weapons else []),'policy':'Explicit constant CreateCharacter MODEL_MONSTER01 binding; dynamic variants omitted'}
   if t in found and found[t]['files']!=row['files']:
    conflicts.append(t)
    if file.name!='GMCrywolf1st.cpp':continue
    row['maps']=[34];row['policy']+='; Crywolf1st binding restricted to map34'
   found[t]=row
for t in set(conflicts):
 if found.get(t,{}).get('maps')!=[34]:found.pop(t,None);actors.pop(str(t),None)
# Preserve previous verified special rates, then native defaults/multipliers.
openData=(a.source/'ZzzOpenData.cpp').read_text('latin1');rateBlock=openData[openData.index('void OpenMonsterModel('):openData.index('void OpenMonsterModels(')]
walkRates={}
for m in re.finditer(r'case\s+(\d+):\s*b->Actions\[MONSTER01_WALK\]\.PlaySpeed\s*=\s*([\d.]+)f',rateBlock):walkRates[int(m[1])]=float(m[2])*25
for t,row in found.items():
 idx=row['monsterModel'];row['textureDirectory']='Npc' if idx in [70,71,72] else 'Object31' if idx==74 else 'Monster';factor=1.2 if idx==3 else .7 if idx in [5,25] else .4 if idx in [37,42] else 1
 for k in ['idleKeys','walkKeys','attackKeys']:row[k]*=factor
 row['walkKeys']=walkRates.get(idx,row['walkKeys'])
 if t in actors and actors[t].get('kind')=='monster':row.update({k:actors[t][k] for k in ['walkKeys','moveSpeed','opacity','altitude'] if k in actors[t]})
 actors[str(t)]=row
# Player-skinned skeletons are explicitly rendered through the Player bone matrices.
for t,skin,scale,right,left in [(14,1,.95,6,6*512+4),(15,2,1.1,-1,4*512+2),(16,3,1.2,512+3,6*512+6),(55,1,1.4,3*512+9,-1),(56,1,.8,3*512+8,-1)]:
 if t not in monsters:continue
 actors[str(t)]={**monsters[t],'kind':'monster','playerMonster':True,'skeletonDeath':True,'files':['Player/Player.bmd',f'Skill/Skeleton{skin:02}.bmd'],'action':0,'weaponRight':right,'weaponLeft':left,'scale':scale,'opacity':1,'altitude':0,'additive':-1,'idleKeys':6.25,'walkKeys':8.25,'attackKeys':8.25,'deathKeys':11.25,'renderWarnings':['Skeleton weapon attachments and death bone fragments remain incomplete'],'bindingSource':{'file':'ZzzCharacter.cpp','types':[14,15,16,55,56]},'policy':'Explicit Player subtype + Skill/Skeleton model binding; source death hides body before bone fragments'}
if 34 in monsters:
 actors['34']={**monsters[34],'kind':'monster','playerMonster':True,'files':['Player/Player.bmd']+['Player/'+part+'Male04.bmd' for part in ['Helm','Armor','Pant','Glove','Boot']],'action':0,'weaponRight':5*512+5,'weaponLeft':6*512+14,'scale':.9,'opacity':1,'altitude':0,'additive':-1,'idleKeys':6.25,'walkKeys':8.25,'attackKeys':8.25,'deathKeys':11.25,'renderWarnings':['Cursed Wizard weapon attachments and +9 visual effect remain incomplete'],'bindingSource':{'file':'ZzzCharacter.cpp','type':34},'policy':'Player model with source armor type3 and SetCharacterScale wizard .9'}
# Native rand()%2 chooses Monster72 or Monster73 for odd Chaos Castle classes.
for t in [163,165,167,169,171,173,427]:
 if t not in monsters:continue
 actors[str(t)]={**monsters[t],'kind':'monster','files':['Monster/Monster72.bmd'],'variantFiles':['Monster/Monster72.bmd','Monster/Monster73.bmd'],'textureDirectory':'Npc','scale':.9,'opacity':1,'altitude':0,'additive':-1,'idleKeys':6.25,'walkKeys':8.5,'attackKeys':8.25,'deathKeys':13.75,'renderWarnings':['Chaos Castle variant weapon attachments remain incomplete'],'bindingSource':{'file':'ZzzCharacter.cpp','types':[163,165,167,169,171,173,427]},'policy':'Native random 71/72 model index; uniform random choice once per viewport actor'}
for t,model,scale,maps,attackable in [(100,'Object2/Object40.bmd',.9,[1],False),(101,'Object2/Object41.bmd',.9,[1],False),(102,'Object2/Object52.bmd',.9,[1],False),(103,'Object5/Object26.bmd',.9,[4],False),(105,'Npc/c_mon.bmd',1,[38],False),(200,'Skill/Ball01.bmd',1.8,[6],True)]:
 if t not in monsters:continue
 actors[str(t)]={**monsters[t],'kind':'monster','files':[model],'scale':scale,'maps':maps,'attackable':attackable,'opacity':1,'altitude':0,'additive':2 if t==200 else -1,'idleKeys':6.25,'walkKeys':8.5,'attackKeys':8.25,'deathKeys':13.75,'bindingSource':{'file':'GM_Kanturu_2nd.cpp' if t==105 else 'ZzzCharacter.cpp','type':t},'policy':'Native direct object/Skill model reference; trap-specific projectile effects not fully ported'}
spawnMaps=[];types=set()
for file in sorted((a.server/'Monster/Spawn').glob('*.txt')):
 m=re.match(r'(\d{3})',file.name)
 if not m:continue
 section=None;spawns=[]
 for line,r in rows(file):
  if r[0]=='end':section=None;continue
  if section is None:section=int(r[0]);continue
  t=int(r[0]);spec=monsters.get(t)
  if not spec or spec['npc']:continue
  types.add(t);spawns.append({'type':t,'name':spec['name'],'section':section,'sourceLine':line,'tokens':r,'mapped':str(t) in actors})
 spawnMaps.append({'map':int(m[1]),'source':file.name,'sha256':hashlib.sha256(file.read_bytes()).hexdigest(),'monsterSpawns':spawns})
catalog['monsterSpawnMaps']=spawnMaps;a.catalog.write_text(json.dumps(catalog,indent=2)+'\n')
a.report.write_text(json.dumps({'mappedBindings':len(found),'configuredTypes':len(types),'unmappedTypes':[monsters[t] for t in sorted(types) if str(t) not in actors],'conflicts':sorted(set(conflicts)),'maps':spawnMaps},indent=2)+'\n');print('Bindings',len(found),'configured',len(types),'mapped',sum(str(t) in actors for t in types),'unmapped',sum(str(t) not in actors for t in types))

# Keep combat classification in sync with the same source-backed bindings.
policy=a.catalog.with_name('monster-policy.js')
if policy.exists():
 marker='// Generated from source-backed actor-catalog.json; model IDs are not an NPC/monster boundary.'
 text=policy.read_text().split(marker)[0].rstrip()
 attackable=sorted(int(k) for k,v in actors.items() if v.get('kind')=='monster' and v.get('attackable') is not False and not v.get('hidden'))
 policy.write_text(text+'\n\n'+marker+'\nexport const attackableMonsterTypes=new Set('+json.dumps(attackable,separators=(',',':'))+');\n')
