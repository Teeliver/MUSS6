from pathlib import Path
import re,json,hashlib
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--source',type=Path,required=True);parser.add_argument('--data',type=Path,required=True);parser.add_argument('--custom-map',type=Path);args=parser.parse_args()
source=args.source.read_text(errors='replace')
data=args.data;files={p.stem.lower():p for p in (data/'Interface/ImgsMapName').glob('*.ozt')}
alias={'dungeun':'Dungeon','stadium':'Arena','tarcan':'Tarkan','loren':'ValleyofLoren','ordeal':'LandofTrial','crywolffortress':'Crywolf','lostkalima':'Kalima','kantru':'Kanturu','balgasbarrack':'BalgassBarracks','balgasrefuge':'BalgassRefuge','mapname_raklion':'Raklion','mapname_raklionboss':'RaklionBoss','pkfield':'Vulcanus','doppelganger':'DoubleGoer','empireguardian':'ImperialGuardian','mapname_marketrolen':'LorenMarket','mapname_karutan':'Karutan'}
out={}
for mapid,name in re.findall(r'm_mapImgPath\[(\d+)\] = strFolderName \+ "([^"]+)"',source):
 stem=Path(name).stem.lower();p=files.get(alias.get(stem,stem).lower());assert p,name
 b=p.read_bytes();out[mapid]={'path':str(p.relative_to(data)),'sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b)}
if args.custom_map:
 for line in args.custom_map.read_text(errors='replace').splitlines():
  m=re.match(r'\s*(\d+)\s+"[^"]*"\s+"([^"]+)"',line)
  if not m: continue
  mapid,name=m.group(1),m.group(2).replace('\\\\','/')
  if mapid=='40': continue  # UIMapName::ShowMapName hides WD_40AREA_FOR_GM
  stem=Path(name).stem.lower();p=files.get(alias.get(stem,stem).lower())
  if p is None: continue
  b=p.read_bytes();out[mapid]={'path':str(p.relative_to(data)),'sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b)}
p=Path(__file__).resolve().parents[1]/'client/src/world/map-title-assets.js';p.write_text('// UIMapName.cpp map IDs; supplied Data Interface/ImgsMapName aliases.\nexport const mapTitleAssets='+json.dumps(out,indent=2)+';\n')
print(len(out),'map bindings',len(set(v['path'] for v in out.values())),'assets')
