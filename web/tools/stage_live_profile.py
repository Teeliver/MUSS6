#!/usr/bin/env python3
"""Stage only user-authorized distributed client fields; never DB/account config."""
import configparser,json,argparse
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('--client',type=Path,required=True);args=parser.parse_args()
config=configparser.ConfigParser();config.read(args.client)
c=config['Client'];version=c['ClientVersion'].replace('.','');
if len(version)!=5 or not version.isascii() or not version.isdigit():raise ValueError('Invalid client version')
serial=c['ClientSerial'];a=int(c['ServerEncDecKey1']);b=int(c['ServerEncDecKey2'])
if len(serial)!=16 or not serial.isascii() or any(not 33<=ord(ch)<=126 for ch in serial) or not 0<=a<=255 or not 0<=b<=255:raise ValueError('Invalid distributed profile')
target=Path(__file__).resolve().parents[1]/'dist/debug/runtime';target.mkdir(parents=True,exist_ok=True)
(target/'live-profile.json').write_text(json.dumps({'version':[ord(ch) for ch in version],'serial':serial,'rawMhp1':a,'rawMhp2':b})+'\n')
print('Staged authorized transport profile into Debug runtime only; no account/DB fields copied')
