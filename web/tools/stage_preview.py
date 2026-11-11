#!/usr/bin/env python3
"""Explicit debug-only staging of original model/texture assets. No config, key or whole-Data exposure."""
import os
import argparse
import importlib.util
import json
import shutil
import subprocess
from pathlib import Path
spec = importlib.util.spec_from_file_location('manifest', Path(__file__).with_name('resource_manifest.py'))
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
PATHS = ['Skill/Laser01.bmd', 'Skill/dragon02.OZJ', 'Effect/JointSpirit01.OZJ', 'Effect/Thunder01.OZJ', 'Skill/Fire01.bmd', 'Skill/fire01.OZJ', 'Skill/fire02.OZJ', 'Local/Eng/Item_eng.bmd', 'Item/Sword01.bmd', 'Item/Wing01.bmd', 'Player/HelmMale01.bmd', 'Player/Player.bmd', 'Item/sword02.OZJ', 'Item/elfin_wing.OZJ', 'Player/head_02.OZJ', 'Player/hide.OZJ', 'Player/skin_barbarian_01.OZJ'] + [f'Player/{part}Class01.bmd' for part in ['Helm', 'Armor', 'Pant', 'Glove', 'Boot']]
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--data', required=True, type=Path)
    parser.add_argument('--music', type=Path)
    parser.add_argument('--reuse-audio-from', type=Path, help='Optional existing generated audio-assets/Data; reuse only identical hashes')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    target = root / 'dist/debug/preview-assets'
    if not (root / 'dist/debug/index.html').is_file():
        raise SystemExit('Build debug first')
    audit = json.loads(subprocess.check_output(['node', str(root / 'tools/body_asset_audit.mjs'), str(args.data)], text=True))
    title_source = (root/'client/src/world/map-title-assets.js').read_text()
    titles = json.loads(title_source.split('export const mapTitleAssets=',1)[1].rsplit(';',1)[0])
    paths = sorted(set(PATHS + audit['paths']))
    document = manifest.generate(args.data, {'entries': [{'path': p} for p in paths]})
    if target.exists():
        shutil.rmtree(target)
    for entry in document['entries']:
        dest = target / 'Data' / entry['path']
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(args.data / entry['path'], dest)
    # Separate exact map-title allowlist; keep the model manifest domain unchanged.
    import hashlib
    for title in titles.values():
        source = (args.data / title['path']).resolve(strict=True)
        if not source.is_relative_to(args.data.resolve()) or not title['path'].startswith('Interface/ImgsMapName/'):
            raise ValueError('Map title escapes Data root')
        content = source.read_bytes()
        if len(content) != title['bytes'] or hashlib.sha256(content).hexdigest() != title['sha256']:
            raise ValueError('Map title baseline mismatch')
        destination = target / 'Data' / title['path']
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(content)
    skill_source=(root/'client/src/ui/skill-catalog.js').read_text()
    skill_assets=json.loads(skill_source.split('export const skillCatalog=',1)[1].rsplit(';',1)[0])['assets']
    for path, asset in skill_assets.items():
        source=(args.data/path).resolve(strict=True)
        if not source.is_relative_to(args.data.resolve()) or not path.startswith('Interface/newui_'):
            raise ValueError('Unsafe skill texture path')
        content=source.read_bytes()
        if len(content)!=asset['bytes'] or hashlib.sha256(content).hexdigest()!=asset['sha256']:
            raise ValueError('Skill texture baseline mismatch')
        destination=target/'Data'/path
        destination.parent.mkdir(parents=True,exist_ok=True)
        destination.write_bytes(content)
    audio_source = (root/'client/src/audio/catalog.js').read_text()
    audio_catalog = json.loads(audio_source.split('export const audioCatalog=',1)[1].rsplit(';',1)[0])
    audio_target = root/'dist/debug/audio-assets/Data'
    for path, sound in audio_catalog['assets'].items():
        source_root = args.music if path.startswith('Music/') and args.music else args.data
        source = (source_root/(path[6:] if source_root==args.music else path)).resolve(strict=True)
        if not source.is_relative_to(source_root.resolve()) or not path.lower().startswith(('sound/','music/')) or source.suffix.lower() not in ['.wav','.mp3','.ogg']:
            raise ValueError('Unsafe audio path')
        content=source.read_bytes()
        if len(content)!=sound['bytes'] or hashlib.sha256(content).hexdigest()!=sound['sha256']:
            raise ValueError('Audio baseline mismatch')
        destination=audio_target/path
        destination.parent.mkdir(parents=True,exist_ok=True)
        reused=False
        if args.reuse_audio_from:
            reuse_root=args.reuse_audio_from.resolve()
            candidate=reuse_root/path
            if candidate.is_file() and candidate.resolve().is_relative_to(reuse_root):
                previous=candidate.read_bytes()
                if len(previous)==sound['bytes'] and hashlib.sha256(previous).hexdigest()==sound['sha256']:
                    try:
                        os.link(candidate,destination)
                        reused=True
                    except OSError:
                        pass
        if not reused:
            destination.write_bytes(content)
    (target / 'manifest.json').write_text(json.dumps(document, indent=2) + '\n')
    (target / 'body-audit.json').write_text(json.dumps(audit, indent=2) + '\n')
    skill_report = json.loads(subprocess.check_output(['python3', str(root / 'tools/skill_class_audit.py'), str(args.data.parent.parent / 'MuServerEX603/Data/Skill/SkillList.txt')], text=True))
    (target / 'skill-menu.json').write_text(json.dumps({'schema': 1, 'sourceHash': skill_report['sha256'], 'skills': skill_report['skills']}, ensure_ascii=True) + '\n')
    print(f'Staged {len(paths)} assets into debug only; original Data unchanged')
