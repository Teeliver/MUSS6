#!/usr/bin/env python3
"""Inventory original files; build participation is evidence, never a parity claim."""
import argparse, collections, hashlib, json, pathlib
p = argparse.ArgumentParser()
p.add_argument('--root', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[4])
p.add_argument('--output', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[1] / 'audit/source-manifest.json')
a = p.parse_args()
root = a.root.resolve()
native = pathlib.Path(__file__).resolve().parents[1]
ninja = native / 'build/build.ninja'
build = ninja.read_text() if ninja.exists() else ''
files, counts = [], collections.Counter()
for f in sorted(root.rglob('*')):
    if not f.is_file() or f.is_relative_to(native) or '.git' in f.parts or f.name == '.DS_Store':
        continue
    rel = f.relative_to(root).as_posix()
    data = f.read_bytes()
    group = '/'.join(f.relative_to(root).parts[:3])
    source = f.suffix.lower() in {'.cpp', '.c', '.h', '.hpp', '.mm', '.m', '.inl'}
    compiled = f.suffix.lower() in {'.cpp', '.c', '.mm', '.m'} and str(f) in build
    files.append(dict(path=rel, bytes=len(data), sha256=hashlib.sha256(data).hexdigest(), source=source,
                      native_build_mentions=bool(compiled), review='not_individually_verified'))
    counts[group] += 1
result = dict(root=str(root), scope='All original files except native output tree, .git and .DS_Store. A build mention does not establish behavioral parity.',
              files_count=len(files), source_count=sum(x['source'] for x in files), native_build_mentions=sum(x['native_build_mentions'] for x in files),
              groups=dict(sorted(counts.items())), files=files)
a.output.parent.mkdir(parents=True, exist_ok=True)
a.output.write_text(json.dumps(result, ensure_ascii=False, indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k not in ('files','groups')}, ensure_ascii=False))
