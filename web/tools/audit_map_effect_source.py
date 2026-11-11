"""Inventory native map/effect call sites; evidence list, never a parity assertion."""
from pathlib import Path
import argparse,re,json,hashlib,collections
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
files=set(a.source.glob('GM*.cpp'))|set(a.source.glob('w_*.cpp'))|{a.source/n for n in ['ZzzObject.cpp','ZzzLodTerrain.cpp','ZzzEffectFireLeave.cpp','ZzzEffectParticle.cpp','MapManager.cpp','ZzzOpenData.cpp']}
patterns=re.compile(r'\b(CreateParticle(?:Sync)?|CreateSprite|CreateEffect(?:Sync)?|CreateJoint(?:Sync)?|CreateFire|AddTerrainLight(?:Clip)?|PlayAnimation|PlayBuffer)\s*\(|\b(BlendMeshTexCoord[UV]|BlendMeshLight|HiddenMesh|StreamMesh)\s*=')
rows=[]
for path in sorted(files):
 if not path.exists():continue
 raw=path.read_bytes();s=raw.decode('utf8',errors='replace');s=re.sub(r'/\*.*?\*/',lambda m:'\n'*m.group().count('\n'),s,flags=re.S);s=re.sub(r'//[^\n]*','',s)
 refs=[];function=None
 for line,text in enumerate(s.splitlines(),1):
  if re.match(r'^\s*(?:void|bool|int|float|BOOL|OBJECT\*)\s+[\w:]+\s*\(',text):function=text.strip()[:180]
  if patterns.search(text):refs.append({'line':line,'function':function,'code':text.strip()[:500]})
 if refs:rows.append({'file':path.name,'sha256':hashlib.sha256(raw).hexdigest(),'mapSymbols':sorted(set(re.findall(r'\bWD_\w+',s))),'references':refs})
output={'scope':'Native source call-site inventory, not verified map-to-call reachability or completed effects','source':str(a.source),'files':rows,'totalReferences':sum(len(r['references']) for r in rows),'status':'Every branch needs port+resource binding+runtime/visual verification; global particle functions include non-map users.'}
a.output.write_text(json.dumps(output,ensure_ascii=False,indent=2)+'\n');print(len(rows),'source files;',output['totalReferences'],'effect/material/animation references')
