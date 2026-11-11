#!/usr/bin/env python3
"""Build an exact-case, hashed model manifest from an audited mapping domain."""
import argparse
import hashlib
import json
import re
from pathlib import Path


def safe_path(value):
    return isinstance(value, str) and bool(re.fullmatch(r'(?:Local/Eng/Item_eng\.bmd|(?:Item|Player|Skill|Effect)/[A-Za-z0-9_-]+\.(?:[bB][mM][dD]|[oO][zZ][jJtT]))', value))


def generate(data, audit):
    root = data.resolve(strict=True)
    paths = sorted({entry['path'] for entry in audit['entries']})
    files = {}
    for path in root.rglob('*'):
        if path.is_file() and path.suffix.lower() in {'.bmd', '.ozj', '.ozt'}:
            files.setdefault(path.relative_to(root).as_posix().lower(), []).append(path)
    entries = []
    for canonical in paths:
        if not safe_path(canonical):
            raise ValueError('Unsafe canonical path')
        matches = files.get(canonical.lower(), [])
        if len(matches) != 1:
            raise ValueError('Missing or ambiguous asset: ' + canonical)
        path = matches[0]
        resolved = path.resolve(strict=True)
        if not resolved.is_relative_to(root) or not resolved.is_file():
            raise ValueError('Asset escapes Data root')
        actual = path.relative_to(root).as_posix()
        if not safe_path(actual):
            raise ValueError('Unsupported exact path')
        before = path.stat()
        if not 0 < before.st_size <= 67108864:
            raise ValueError('Unsupported asset size')
        digest = hashlib.sha256()
        with path.open('rb') as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                digest.update(chunk)
        after = path.stat()
        if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns):
            raise ValueError('Asset changed during hashing')
        entries.append(dict(canonical=canonical, path=actual, size=after.st_size, sha256=digest.hexdigest()))
    if not entries or len(entries) > 8192:
        raise ValueError('Invalid manifest domain')
    payload = json.dumps(entries, sort_keys=True, separators=(',', ':')).encode()
    return dict(schema=1, version=hashlib.sha256(payload).hexdigest(), scope='native mapped item models only', entries=entries)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--data', type=Path, required=True)
    parser.add_argument('--audit', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = generate(args.data, json.loads(args.audit.read_text()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'models': len(result['entries']), 'bytes': sum(e['size'] for e in result['entries']), 'caseCorrections': sum(e['canonical'] != e['path'] for e in result['entries']), 'version': result['version']}))
