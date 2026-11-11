#!/usr/bin/env python3
"""Offline, read-only audit of EX603 text catalogs; never opens ServerInfo.sse/config credentials."""
import argparse
import hashlib
import json
import re
import shlex
from pathlib import Path


def parse_catalog(text, kind, capacity):
    rows, indices, items = [], set(), set()
    ended = False
    for line_no, line in enumerate(text.splitlines(), 1):
        content = line.split('//', 1)[0]
        lexer = shlex.shlex(content, posix=True)
        lexer.whitespace += ','
        lexer.whitespace_split = True
        lexer.commenters = ''
        tokens = list(lexer)
        if not tokens:
            continue
        if ended:
            raise ValueError(f'Content after end at line {line_no}')
        if tokens == ['end']:
            ended = True
            continue
        expected = 24 if kind == 'wing' else 6
        if len(tokens) != expected:
            raise ValueError(f'Wrong {kind} schema at line {line_no}')
        nums = [int(value, 10) for value in tokens[:-1]]
        if kind == 'wing':
            index, group, item = nums[:3]
        else:
            index, (group, item) = len(rows), nums[:2]
            if any(not 0 <= c <= 255 for c in nums[2:5]):
                raise ValueError('Invalid RGB')
        stem = tokens[-1]
        if not re.fullmatch(r'[A-Za-z0-9_-]{1,31}', stem):
            raise ValueError('Unsupported or unsafe model stem')
        if not 0 <= index < capacity or not 0 <= group <= 15 or not 0 <= item < 512:
            raise ValueError('Catalog index/item outside EX603 bounds')
        item_index = group * 512 + item
        if index in indices or item_index in items:
            raise ValueError('Duplicate catalog index/item')
        indices.add(index)
        items.add(item_index)
        directory = 'Item' if kind == 'wing' or not 7 <= group < 12 else 'Player'
        rows.append(dict(index=index, itemIndex=item_index, model=f'{directory}/{stem}.bmd'))
    if not ended:
        raise ValueError('Missing end marker')
    return rows


def inspect_assets(rows, data):
    files = {}
    for path in data.rglob('*'):
        if path.is_file() and path.suffix.lower() == '.bmd':
            files.setdefault(path.relative_to(data).as_posix().lower(), []).append(path.relative_to(data).as_posix())
    result = []
    for row in rows:
        matches = files.get(row['model'].lower(), [])
        status = 'missing' if not matches else 'ambiguous-case' if len(matches) > 1 else 'exact' if matches[0] == row['model'] else 'case-mismatch'
        result.append(dict(**row, assetStatus=status, matches=matches))
    return result


def wing_code(index):
    return ((index + 1) << 1) & 255


def audit(server, client, data):
    report = {'schema': 1, 'scope': 'custom text catalogs only; no builtin model/texture validation', 'catalogs': {}}
    specs = [('serverWing', server, 'wing', 50), ('clientWing', client / 'CustomWing.txt', 'wing', 100), ('clientItem', client / 'CustomItem.txt', 'item', 2000)]
    parsed = {}
    for name, path, kind, capacity in specs:
        raw = path.read_bytes()
        rows = parse_catalog(raw.decode('utf-8-sig'), kind, capacity)
        parsed[name] = rows
        report['catalogs'][name] = {'sha256': hashlib.sha256(raw).hexdigest(), 'count': len(rows), 'entries': inspect_assets(rows, data)}
    by_index = lambda rows: {r['index']: (r['itemIndex'], r['model']) for r in rows}
    report['wingTextCatalogsAgree'] = by_index(parsed['serverWing']) == by_index(parsed['clientWing'])
    builtin = {n << 5: 129 + n for n in range(1, 7)}
    report['activeWingBuiltinCollisions'] = [{'index': r['index'], 'builtinWingType': builtin[wing_code(r['index'])]} for r in parsed['serverWing'] if wing_code(r['index']) in builtin]
    report['potentialServerWingBuiltinCollisions'] = [{'index': i, 'encodedByte17': wing_code(i), 'builtinWingType': builtin[wing_code(i)]} for i in range(50) if wing_code(i) in builtin]
    report['compiledClientCatalogVerified'] = False
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--server-wing', required=True, type=Path)
    parser.add_argument('--client-catalogs', required=True, type=Path)
    parser.add_argument('--data', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    result = audit(args.server_wing, args.client_catalogs, args.data)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'counts': {name: value['count'] for name, value in result['catalogs'].items()}, 'textCatalogsAgree': result['wingTextCatalogsAgree'], 'potentialCollisions': result['potentialServerWingBuiltinCollisions']}))
