"""Read-only structural audit. Schema adapted from native/client/assets.cpp.
No renderer mapping or per-world tile policy is inferred for unsupported worlds.
"""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import re
import struct

KEY = bytes.fromhex('D1 73 52 F6 D2 9A CB 27 3E AF 59 31 37 B3 E7 A2')
SUPPORTED = {1, 4, 52, 74, 75}


def decrypt(data):
    result = bytearray(len(data))
    previous = 0x5e
    for i, value in enumerate(data):
        result[i] = ((value ^ KEY[i % 16]) - previous) & 255
        previous = (value + 0x3d) & 255
    return result


def check(ok, message):
    if not ok:
        raise ValueError(message)


def object_layout(data, world):
    b = decrypt(data)
    check(len(b) >= 4 and b[0] == 0 and b[1] == world, 'object header/world mismatch')
    check((len(b) - 4) % 30 == 0, 'incomplete object record')
    declared = struct.unpack_from('<H', b, 2)[0]
    records = (len(b) - 4) // 30
    types = Counter()
    active = 0
    for index, record in enumerate(struct.iter_unpack('<h7f', memoryview(b)[4:])):
        kind, *values = record
        check(0 <= kind < 4096, 'object type outside native schema')
        check(all(math.isfinite(v) and abs(v) < 1e7 for v in values), f'invalid placement float at record {index}: {values}')
        x, y, z, rx, ry, rz, scale = values
        check(0 <= scale <= 100, f'scale outside native schema at record {index}: {scale}')
        if scale == 0:
            check(x == y == z == 0, 'nonzero position with zero scale')
        else:
            check(-10000 <= x <= 40000 and -10000 <= y <= 40000, 'position outside native bounds')
            active += 1
            types[kind] += 1
    return dict(declared=declared, records=records, active=active,
                declared_delta=declared-records, types=dict(sorted(types.items())),
                native_count_compatible=records <= 10000 and declared in (records, records+1))


def mapping_layout(data, world):
    b = decrypt(data)
    check(len(b) == 196610, 'mapping size differs from native schema')
    check(b[:2] == bytes((0, world)), 'mapping header/world mismatch')
    return dict(base_indices=sorted(set(b[2:65538])), overlay_indices=sorted(set(b[65538:131074])),
                samples=65536, tile_mapping_verified=False)


def attribute_layout(data, world):
    b = decrypt(data)
    check(len(b) in (65540, 131076), 'attribute size differs from native schema')
    mask = bytes.fromhex('FC CF AB')
    b = bytes(value ^ mask[i % 3] for i, value in enumerate(b))
    check(b[:4] == bytes((0, world, 255, 255)), 'attribute header/world mismatch')
    return dict(samples=65536, bits=16 if len(b) == 131076 else 8)


def height_layout(b, world):
    check(len(b) >= 58 and b[4:6] == b'BM', 'unsupported height wrapper')
    header, width, height = struct.unpack_from('<Iii', b, 18)
    planes, bits, compression = struct.unpack_from('<HHI', b, 30)
    check((header, width, height, planes, bits, compression) == (40,256,256,1,8,0),
          f'height differs from native schema: header={header}, width={width}, height={height}, planes={planes}, bits={bits}, compression={compression}')
    offset = struct.unpack_from('<I', b, 14)[0] + 4
    check(offset >= 58 and (offset+65536 == len(b) or 1084+65536 == len(b)), 'height pixel size mismatch')
    check(0 <= 1084-offset <= 2 and 1084 < len(b), f'height legacy offset incompatible: BMP pixel offset={offset}, native=1084')
    return dict(samples=65536, bmp_offset=offset, native_offset=1084,
                native_tail_padding=max(0, 1084+65536-len(b)), visual_alignment_verified=False)


def audit(root):
    worlds = []
    for directory in sorted(root.iterdir()):
        match = re.fullmatch(r'World([1-9][0-9]*)', directory.name)
        if not match or directory.is_symlink() or not directory.is_dir():
            continue
        world = int(match[1])
        row = dict(world=world, native_world_supported=world in SUPPORTED, files=[])
        worlds.append(row)
        for filename, decoder in ((f'EncTerrain{world}.obj', object_layout),
                                  (f'EncTerrain{world}.map', mapping_layout),
                                  (f'EncTerrain{world}.att', attribute_layout),
                                  ('TerrainHeight.OZB', height_layout)):
            path = directory / filename
            entry = dict(path=path.relative_to(root).as_posix())
            row['files'].append(entry)
            try:
                check(path.is_file() and not path.is_symlink(), 'missing regular file (symlinks excluded)')
                check(0 < path.stat().st_size <= 64*1024*1024, 'file size outside audit bound')
                data = path.read_bytes()
                entry['sha256'] = hashlib.sha256(data).hexdigest()
                entry['layout'] = decoder(data, world)
                entry['status'] = 'schema_parsed'
            except (ValueError, OSError, struct.error) as error:
                entry['status'] = 'schema_error'
                entry['error'] = str(error)
    return dict(worlds=worlds, counts=dict(Counter(f['status'] for w in worlds for f in w['files'])),
                limits=['Structural schema inspection only; no new world enabled in native client.',
                        'No model naming, texture mapping, collision, lighting or visual parity validated.',
                        'Schema comes from current native adapter; complete Windows EX603 engine is absent.'])


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--data', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    report = audit(a.data.resolve(strict=True))
    with a.output.open('x') as f:
        json.dump(report, f, ensure_ascii=False, indent=2)
        f.write('\n')
    print(json.dumps(report['counts']))

if __name__ == '__main__':
    main()
