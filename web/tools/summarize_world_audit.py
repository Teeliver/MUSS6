"""Combine offline reports; classifies unknown tables by path, never claims decoding."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re


def category(path):
    if '/Minimap/' in path or re.fullmatch(r'World\d+/[Mm]inimap.bmd', path):
        return 'minimap_by_path'
    if re.fullmatch(r'Local/(Eng|Por|Spn)/(Text|Item|Skill)_(eng|por|spn).bmd', path):
        return 'supported_local_table'
    if path.startswith('Local/'):
        return 'other_local_table_by_path'
    return 'unclassified_non_model'


def summarize(world_report, asset_rows):
    out = dict(world_report)
    out['worlds'] = sorted(out['worlds'], key=lambda r: r['world'])
    tables = {r['path']: r for r in out['tables']}
    out['non_model_bmd'] = [dict(path=r['path'], category=category(r['path']),
        status=tables.get(r['path'], {}).get('status', 'not_decoded'))
        for r in asset_rows if r['kind'] == 'non_model_bmd']
    out['classification_counts'] = dict(Counter(r['category'] for r in out['non_model_bmd']))
    out['limits'] = ['Classification by path is not format validation.',
        'World dependency names/marker exclusions follow current native world_view.mm, not a proven complete Windows mapping.',
        'No render, collision, animation, terrain texture fallback or browser validation.',
        'Worlds unsupported by native decoder remain unaudited.']
    return out


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--worlds', type=Path, required=True)
    p.add_argument('--assets', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    result = summarize(json.loads(a.worlds.read_text()), [json.loads(s) for s in a.assets.read_text().splitlines()])
    result['input_sha256'] = {k: hashlib.sha256(v.read_bytes()).hexdigest() for k,v in [('worlds', a.worlds), ('assets_W0002', a.assets)]}
    # Errors need only relative resource names in the shared report.
    for world in result['worlds']:
        for key in ('terrain', 'objects'):
            if 'error' in world.get(key, {}):
                world[key]['error'] = re.sub(r'/Users/[^:]+/Data/', '', world[key]['error'])
    with a.output.open('x') as f:
        json.dump(result, f, ensure_ascii=False, indent=2)
        f.write('\n')

if __name__ == '__main__':
    main()
