from pathlib import Path
import argparse,re,ast,operator,json,hashlib
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--data',type=Path,required=True);p.add_argument('--music',type=Path);a=p.parse_args();root=Path(__file__).resolve().parents[1]
constants={}
def evaluate(expr):
 n=ast.parse(expr.strip(),mode='eval').body
 def visit(n):
  if isinstance(n,ast.Constant) and type(n.value)==int:return n.value
  if isinstance(n,ast.Name):return constants[n.id]
  if isinstance(n,ast.UnaryOp) and isinstance(n.op,ast.USub):return -visit(n.operand)
  if isinstance(n,ast.BinOp) and type(n.op) in [ast.Add,ast.Sub,ast.Mult,ast.LShift,ast.BitOr]:return {ast.Add:operator.add,ast.Sub:operator.sub,ast.Mult:operator.mul,ast.LShift:operator.lshift,ast.BitOr:operator.or_}[type(n.op)](visit(n.left),visit(n.right))
  raise ValueError('Nonliteral expression')
 return visit(n)
header=(a.source/'DSPlaySound.h').read_text(errors='replace')+'\n'+(a.source/'_enum.h').read_text(errors='replace')
for _ in range(8):
 for name,value in re.findall(r'^\s*#define\s+((?:SOUND|MUSIC)_\w+)\s+([^\r\n/]+)',header,re.M):
  try:constants[name]=evaluate(value)
  except (ValueError,SyntaxError,KeyError):pass
enum_header=re.sub(r'/\*.*?\*/','',header,flags=re.S);enum_header=re.sub(r'//[^\n]*','',enum_header);enum_header=re.sub(r'^\s*#.*$','',enum_header,flags=re.M)
for block in re.findall(r'enum(?:\s+\w+)?\s*\{(.*?)\}',enum_header,re.S):
 previous=-1
 for entry in block.split(','):
  entry=entry.strip()
  if not entry:continue
  try:
   if '=' in entry:name,value=entry.split('=',1);previous=evaluate(value)
   else:name=entry;previous+=1
   if re.fullmatch(r'(?:SOUND|MUSIC)_\w+',name.strip()):constants[name.strip()]=previous
  except (ValueError,SyntaxError,KeyError):pass
assets={};lookup={}
for f in a.data.rglob('*'):
 if f.is_file() and f.suffix.lower() in ['.wav','.mp3','.ogg']:
  path=f.relative_to(a.data).as_posix()
  if not path.lower().startswith(('sound/','music/')):continue
  b=f.read_bytes();assets[path]={'bytes':len(b),'sha256':hashlib.sha256(b).hexdigest()};lookup.setdefault(path.lower(),[]).append(path)
if a.music:
 for f in a.music.rglob('*'):
  if f.is_file() and f.suffix.lower() in ['.mp3','.ogg']:
   path='Music/'+f.relative_to(a.music).as_posix();content=f.read_bytes();assets[path]={'bytes':len(content),'sha256':hashlib.sha256(content).hexdigest()};lookup.setdefault(path.lower(),[]).append(path)
def resolve(s):
 value=s.replace('\\\\','/').replace('\\','/');value=re.sub(r'^data/','',value,flags=re.I);x=lookup.get(value.lower(),[]);return x[0] if len(x)==1 else None
bindings={};loads=[];calls=[];music=[];monsters={}
for file in sorted(a.source.glob('*')):
 if file.suffix.lower() not in ['.cpp','.h']:continue
 original=file.read_text(errors='replace');text=re.sub(r'/\*.*?\*/',lambda m:'\n'*m[0].count('\n'),original,flags=re.S);text=re.sub(r'//[^\n]*','',text)
 for m in re.finditer(r'LoadWaveFile\(\s*([^,]+),\s*"([^"]+)"([^;]*?)\)\s*;',text):
  expr,path,tail=m.groups();actual=resolve(path);record={'source':file.name,'line':text.count('\n',0,m.start())+1,'expression':expr.strip(),'requested':path,'path':actual}
  try:record['id']=evaluate(expr)
  except (ValueError,SyntaxError,KeyError):pass
  loads.append(record)
  if actual and 'id' in record:
   args=[v.strip() for v in tail.lstrip().lstrip(',').split(',')];entry={'path':actual,'channels':int(args[0]) if args and args[0].isdigit() else 2,'spatial':len(args)>1 and args[1] in ['true','TRUE','1','Enable','Enable3DSound']}
   rows=bindings.setdefault(str(record['id']),[])
   if entry not in rows:rows.append(entry)
 for m in re.finditer(r'\b(PlayBuffer|StopBuffer|PlayMp3|StopMp3)\s*\(([^;]*?)\)\s*;',text):calls.append({'source':file.name,'line':text.count('\n',0,m.start())+1,'function':m[1],'arguments':m[2].strip()})
 if file.name=='ZzzOpenData.cpp':
  section=text[text.index('int Channel = 2;'):];cases=[]
  for line in section.splitlines():
   m=re.search(r'^\s*case\s+(\d+)\s*:',line)
   if m:cases.append(int(m[1]))
   if re.search(r'\bbreak\s*;',line):cases=[]
   m=re.search(r'SetMonsterSound\(MODEL_MONSTER01\s*\+\s*Type\s*,([^;]+)\);',line)
   if m:
    try:
     values=[evaluate(v) for v in m[1].split(',')]
     for case in cases:monsters[str(case)]=values
    except (ValueError,SyntaxError,KeyError):pass
 if file.name=='ZzzScene.cpp':
  block=re.search(r'char\* g_lpszMp3\[NUM_MUSIC\]\s*=\s*\{(.*?)\};',text,re.S)
  if block:
   for index,path in enumerate(re.findall('"([^"]+)"',block[1])):music.append({'index':index,'requested':path,'path':resolve(path)})
report={'assets':len(assets),'bytes':sum(v['bytes'] for v in assets.values()),'waveLoads':loads,'calls':calls,'music':music,'ambiguousIds':{k:v for k,v in bindings.items() if len({e['path'] for e in v})>1},'missingLoads':[v for v in loads if not v['path']]}
(root/'docs/AUDIO-SOURCE-AUDIT.json').write_text(json.dumps(report,indent=2)+'\n')
(root/'client/src/audio/catalog.js').write_text('// Generated from original source/Data by audit_audio.py.\nexport const audioCatalog='+json.dumps({'assets':assets,'bindings':bindings,'constants':constants,'monsters':monsters,'music':music},separators=(',',':'))+';\n')
print(json.dumps({k:report[k] for k in ['assets','bytes']}), 'loads',len(loads),'calls',len(calls),'missing',len(report['missingLoads']),'ambiguous IDs',len(report['ambiguousIds']))
