"""Extract supplied direct NPC model bindings and all server spawn coverage. Read-only."""
import argparse, hashlib, json, re, shlex
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--server',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
root=Path(__file__).resolve().parents[2]
def lines(path):
 for n,line in enumerate(path.read_text('latin1').splitlines(),1):
  text=line.split('//')[0].strip()
  if text:yield n,shlex.split(text)
monsters={int(r[0]):{'type':int(r[0]),'npc':int(r[1])==1,'name':r[2]} for _,r in lines(a.server/'Monster/MonsterList.txt') if r[0]!='end'}
open_bytes=(a.source/'ZzzOpenData.cpp').read_bytes();char_bytes=(a.source/'ZzzCharacter.cpp').read_bytes();open_text=open_bytes.decode('latin1');char_text=char_bytes.decode('latin1');bindings={}
texture_folders={}
for m in re.finditer(r'OpenTexture\(\s*([A-Z][A-Z0-9_]+)\s*,\s*"([^"]+)"',open_text):
 folder='/'.join(x for x in m[2].replace('\\','/').split('/') if x)
 if re.fullmatch(r'(?:Npc|Monster|Object[1-9][0-9]*)(?:/[A-Za-z0-9_-]+){0,3}',folder):texture_folders[m[1]]=folder
for m in re.finditer(r'AccessModel\(\s*([A-Z][A-Z0-9_]+)\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*(?:,\s*(\d+))?\s*\)',open_text):
 folder='/'.join(x for x in m[2].replace('\\','/').split('/') if x)
 if not re.fullmatch(r'Data/(?:Npc|Monster)(?:/[A-Za-z0-9_-]+){0,3}',folder) and not re.fullmatch(r'Data/Object[1-9][0-9]*',folder):continue
 suffix=f'{int(m[4]):02}' if m[4] else ''
 bindings[m[1]]={'path':folder.removeprefix('Data/')+'/'+m[3]+suffix+'.bmd','line':open_text.count('\n',0,m.start())+1}
actors={}
for m in re.finditer(r'\n\tcase (\d+):([\s\S]*?)(?=\n\tcase |\Z)',char_text[char_text.index('CHARACTER* CreateMonster('):]):
 t=int(m[1]);b=re.search(r'OpenNpc\(([A-Z][A-Z0-9_]+)\)',m[2])
 if t not in monsters or not monsters[t]['npc'] or not b or b[1] not in bindings:continue
 binding=bindings[b[1]];scale=re.search(r'(?:o->Scale|c->Object.Scale)\s*=\s*(\d+(?:\.\d*)?)[fF]?\s*;',m[2]);blend=re.search(r'->BlendMesh\s*=\s*(\d+)',m[2])
 height=re.search(r'(?:o->Position|c->Object.Position)\[2\]\s*(\+=|-=)\s*(\d+(?:\.\d*)?)[fF]?\s*;',m[2]);altitude=(float(height[2])/100*(-1 if height[1]=='-=' else 1)) if height else 0
 if not height:
  h=re.search(r'Position\[2\]\s*=\s*RequestTerrainHeight[^;\n]*\+\s*(\d+(?:\.\d*)?)[fF]?\s*;',m[2])
  if h:altitude=float(h[1])/100
 idle=re.search(r'Actions\[0\]\.PlaySpeed\s*=\s*(\d+(?:\.\d*)?)[fF]?\s*;',m[2])
 actors[t]={**monsters[t],'files':[binding['path']],'textureDirectory':texture_folders.get(b[1]),'idleKeys':float(idle[1])*25 if idle else 6.25,'scale':float(scale[1]) if scale else 1,'altitude':altitude,'hidden':bool(re.search(r'(?:HiddenMesh\s*=\s*-2|Visible\s*=\s*false|Live\s*=\s*false)',m[2])),'opacity':1,'additive':int(blend[1]) if blend else -1,'bindingSource':{'file':'ZzzOpenData.cpp','line':binding['line'],'symbol':b[1]},'policy':'direct supplied source binding; full action/scale policy unverified'}
native=(root/'native/client/entity_assets.mm').read_text()
for m in re.finditer(r'\{(\d+),"([^"]+)",true,([\d.]+),([\d.]+),([\d.]+),[\d.]+,[\d.]+,\{([^}]+)\}\}',native):
 t=int(m[1]);actors[t]={**monsters.get(t,{'type':t,'npc':True,'name':m[2]}),'files':re.findall(r'"([^"]+)"',m[6]),'idleKeys':6.25,'scale':float(m[3]),'opacity':float(m[4]),'altitude':float(m[5]),'additive':1 if t in (238,242) else -1,'bindingSource':{'file':'native/client/entity_assets.mm'},'policy':'native explicit binding including multipart; not Windows parity'}
object_bytes=(a.source/'ZzzObject.cpp').read_bytes();object_text=object_bytes.decode('latin1').replace('\r\n','\n')
raul=re.search(r'else if \(o->Type == MODEL_UNITEDMARKETPLACE_RAUL\)\s*\{([\s\S]*?)\n\s*\}\n\s*else if',object_text)
bright=[int(n) for n in re.findall(r'RenderMesh\((\d+),\s*RENDER_TEXTURE\s*\|\s*RENDER_BRIGHT',raul[1] if raul else '')]
if bright!=[4,14]:raise ValueError('Raul render policy source changed')
if 546 in actors:
 actors[546]['additiveMeshes']=bright
 actors[546]['renderWarnings']=['Raul: shared-asset lookup adapter, chưa xác nhận routing của engine gốc']
if 407 in actors:
 actors[407]['tint']=[0.5,0.5,0.8]
 actors[407]['renderWarnings']=['Quarrel: source chrome overlay chưa triển khai']
# Explicit unarmed Player-based helpers. Validate the supplied source before emitting.
helper=re.search(r'case 258:\s*case 371:\s*case 414:([\s\S]*?)break;',char_text)
if not helper or 'MODEL_PLAYER' not in helper[1] or 'c->Weapon[0].Type = -1;' not in helper[1]:
 raise ValueError('Unarmed Player helper source changed')
for part,macro in [('HELM','HELM'),('ARMOR','ARMOR'),('PANTS','PANTS'),('GLOVES','GLOVES'),('BOOTS','BOOTS')]:
 if not re.search(r'BODYPART_'+part+r'\]\.Type\s*=\s*MODEL_'+macro+r'\s*\+\s*9;',helper[1]):raise ValueError('Helper armor source changed')
for t in (258,371,414):
 if t not in monsters or not monsters[t]['npc']:continue
 actors[t]={**monsters[t],'files':['Player/Player.bmd']+['Player/'+part+'Male10.bmd' for part in ('Helm','Armor','Pant','Glove','Boot')],'action':1,'idleKeys':6.25,'scale':0.9,'altitude':0,'opacity':1,'additive':-1,'bindingSource':{'file':'ZzzCharacter.cpp','line':char_text.count('\n',0,helper.start())+1},'policy':'Unarmed MODEL_PLAYER helper, source armor+9/OpenPlayers; male idle1; PC-room effects not implemented'}
spawns=[]
for file in sorted((a.server/'Monster/Spawn').glob('*.txt')):
 match=re.match(r'(\d{3})',file.name)
 if not match:continue
 section=None;entries=[]
 for line,r in lines(file):
  if r[0]=='end':section=None;continue
  if section is None:section=int(r[0]);continue
  if section==5:continue
  if len(r)<5:raise ValueError(f'Invalid spawn row: {file.name}:{line}')
  t=int(r[0]);spec=monsters.get(t)
  if not spec or not spec['npc']:continue
  direction=r[6] if section==1 else r[4]
  entries.append({'type':t,'name':spec['name'],'section':section,'radius':int(r[1]),'x':int(r[2]),'y':int(r[3]),'direction':None if direction=='*' else int(direction),'sourceLine':line,'mapped':t in actors})
 spawns.append({'map':int(match[1]),'source':file.name,'sha256':hashlib.sha256(file.read_bytes()).hexdigest(),'npcSpawns':entries})
a.output.write_text(json.dumps({'schema':1,'source':{'openDataSha256':hashlib.sha256(open_bytes).hexdigest(),'characterSha256':hashlib.sha256(char_bytes).hexdigest(),'objectSha256':hashlib.sha256(object_bytes).hexdigest()},'actors':{str(k):v for k,v in sorted(actors.items())},'maps':spawns},indent=2)+'\n')
print(f'{len(actors)} explicit NPC bindings; {len(spawns)} spawn maps; {sum(len(m["npcSpawns"]) for m in spawns)} NPC placements (server config, not live instances)')
