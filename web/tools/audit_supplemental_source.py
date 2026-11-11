"""Read-only inventory of supplied client source; no claim of build/version parity."""
import argparse
import hashlib
import json
import re
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
root = args.source.resolve()
files = []
anchors = re.compile(r'\b(MovePath|PathFinding2|CharacterMoveSpeed|SetPlayerWalk|SetPlayerStop|MODEL_FIRE|MODEL_LASER|BITMAP_JOINT_SPIRIT2?|CreateEffectSync|CreateJointSync|CreateParticleSync)\b')
for path in sorted(root.rglob('*')):
    if not path.is_file() or path.suffix.lower() not in {'.cpp', '.h', '.hpp'}:
        continue
    data = path.read_bytes()
    # Latin-1 preserves every byte for line/symbol discovery; not a source rewrite.
    lines = data.decode('latin1').splitlines()
    includes = []
    matches = []
    for number, line in enumerate(lines, 1):
        include = re.match(r'\s*#\s*include\s*[<"]([^>"]+)', line)
        if include:
            name = include.group(1)
            includes.append({'name': name, 'relativeFilePresent': (path.parent / name).is_file()})
        symbols = sorted(set(anchors.findall(line)))
        if symbols:
            matches.append({'line': number, 'symbols': symbols})
    files.append({'path': path.relative_to(root).as_posix(), 'bytes': len(data),
                  'sha256': hashlib.sha256(data).hexdigest(), 'lines': len(lines),
                  'includes': includes, 'anchors': matches})
result = {'schema': 1, 'classification': 'supplied supplemental reference; EX603 version unverified',
          'fileCount': len(files), 'totalBytes': sum(f['bytes'] for f in files), 'files': files}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(result, indent=2) + '\n')
print(f"Inventoried {len(files)} source files / {result['totalBytes']} bytes; original files unchanged")
