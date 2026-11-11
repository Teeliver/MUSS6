#!/usr/bin/env python3
"""Compare native-adapter path domain with Data, without opening configs or modifying assets."""
import argparse
import collections
import json
import subprocess
import tempfile
from pathlib import Path


def audit(data):
    root = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix='mu-resource-audit-') as temp:
        exe = Path(temp) / 'inventory'
        subprocess.run(['clang++', '-std=c++17', '-I', str(root / 'client-core/include'), str(root / 'web/tests/item_resources_contract.cpp'), '-o', str(exe)], check=True)
        lines = subprocess.check_output([str(exe), '--inventory'], text=True).splitlines()
    files = collections.defaultdict(list)
    for path in data.rglob('*'):
        if path.is_file() and path.suffix.lower() == '.bmd':
            rel = path.relative_to(data).as_posix()
            files[rel.lower()].append(rel)
    entries, counts = [], collections.Counter()
    for line in lines:
        index, level, path = line.split('\t')
        matches = files[path.lower()]
        status = 'missing' if not matches else 'ambiguous-case' if len(matches) > 1 else 'exact' if matches[0] == path else 'case-mismatch'
        counts[status] += 1
        entries.append(dict(index=int(index), level=int(level), path=path, status=status, matches=matches))
    return dict(scope='adapter mapping domain 8192 ids x levels 0/1/4/15, not active Item.bmd catalog', counts=dict(counts), mappedCases=len(entries), uniquePaths=len({e['path'] for e in entries}), entries=entries)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--data', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = audit(args.data)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({k: v for k, v in result.items() if k != 'entries'}))
