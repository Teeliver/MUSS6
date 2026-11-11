"""Investigate W0004 schema failures without weakening the production decoder."""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import struct
from world_structure import check, decrypt


def inspect_height(data):
    if not any(data):
        return dict(status='all_zero_file', bytes=len(data), usable_height=False)
    check(len(data) >= 58 and data[4:6] == b'BM', 'unknown wrapper')
    size, offset = struct.unpack_from('<I', data, 6)[0], struct.unpack_from('<I', data, 14)[0] + 4
    header, width, height, planes, bits, compression = struct.unpack_from('<IiiHHI', data, 18)
    result = dict(width=width, height=height, bits=bits, compression=compression,
                  bmp_offset=offset, declared_bmp_bytes=size, actual_bmp_bytes=len(data)-4,
                  game_height_conversion_verified=False)
    check(header == 40 and width == 256 and abs(height) == 256 and planes == 1,
          'unsupported DIB dimensions/header')
    check(bits in (8,24) and compression == 0, 'unsupported pixel encoding')
    row_bytes = ((width*bits+31)//32)*4
    end = offset+row_bytes*abs(height)
    check(offset >= 58, 'pixel offset overlaps header')
    if bits == 8:
        colors = struct.unpack_from('<I',data,50)[0] or 256
        check(1 <= colors <= 256 and offset >= 58+colors*4, 'invalid palette span')
        result['palette_entries'] = colors
    result.update(required_pixel_end=end, available_bytes=len(data))
    if end > len(data):
        result.update(status='truncated_bmp_pixels', missing_pixel_bytes=end-len(data), usable_height=False)
        return result
    # Decode storage rows into top-to-bottom samples without deriving MU heights.
    rows = [data[offset+y*row_bytes:offset+y*row_bytes+width*(bits//8)] for y in range(abs(height))]
    if height > 0:
        rows.reverse()
    pixels = b''.join(rows)
    result.update(status='bmp_pixels_decoded', decoded_pixel_bytes=len(pixels),
                  pixel_sha256=hashlib.sha256(pixels).hexdigest(), trailing_bytes=len(data)-end,
                  declared_size_matches=size==len(data)-4, usable_height=False)
    return result


def inspect_objects(data, world):
    b = decrypt(data)
    check(len(b) >= 4 and b[:2] == bytes((0,world)) and (len(b)-4)%30 == 0, 'unknown object layout')
    issues = []
    records = (len(b)-4)//30
    for index, record in enumerate(struct.iter_unpack('<h7f',memoryview(b)[4:])):
        kind, *v = record
        failures = []
        if not 0 <= kind < 4096:
            failures.append('type_outside_native_bounds')
        if not all(math.isfinite(x) for x in v):
            failures.append('nonfinite_float')
        if any(math.isfinite(x) and abs(x) >= 1e7 for x in v):
            failures.append('float_outside_native_bounds')
        if math.isfinite(v[6]) and not 0 <= v[6] <= 100:
            failures.append('scale_outside_native_bounds')
        if v[6] == 0 and any(x != 0 for x in v[:3]):
            failures.append('zero_scale_nonzero_position')
        if v[6] != 0 and any(math.isfinite(x) and not -10000 <= x <= 40000 for x in v[:2]):
            failures.append('position_outside_native_bounds')
        if failures:
            issues.append(dict(record=index, byte_offset=4+index*30, type=kind,
                values=[x if math.isfinite(x) else str(x) for x in v], reasons=failures))
    return dict(status='placement_policy_issues', records=records, affected_records=len(issues), issues=issues,
                reason_counts=dict(Counter(reason for issue in issues for reason in issue['reasons'])),
                interpretation_verified=False)


def investigate(root, previous):
    results = []
    for world in previous['worlds']:
        for file in world['files']:
            if file['status'] != 'schema_error':
                continue
            relative = Path(file['path'])
            check(not relative.is_absolute() and '..' not in relative.parts, 'unsafe input report path')
            path = root / relative
            check(not path.is_symlink() and path.is_file() and path.resolve().is_relative_to(root), 'unsafe resource path')
            check(path.stat().st_size <= 64*1024*1024, 'resource exceeds audit bound')
            data = path.read_bytes()
            digest = hashlib.sha256(data).hexdigest()
            check(digest == file['sha256'], 'Data changed since W0004: '+relative.as_posix())
            row = dict(path=relative.as_posix(), sha256=digest, input_unchanged=True)
            try:
                row['findings'] = inspect_height(data) if path.suffix.lower() == '.ozb' else inspect_objects(data,world['world'])
            except (ValueError,struct.error) as e:
                row['findings'] = dict(status='unclassified', error=str(e))
            results.append(row)
    return dict(files=results, counts=dict(Counter(r['findings']['status'] for r in results)),
        limits=['No production changes or resource repairs.',
                'BMP pixel decoding does not define MU height conversion or legacy offset policy.',
                'Negative scale semantics and nonfinite placement handling need original engine evidence.'])


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--data',type=Path,required=True)
    p.add_argument('--previous',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a = p.parse_args()
    report = investigate(a.data.resolve(strict=True),json.loads(a.previous.read_text()))
    report['input_report_sha256'] = hashlib.sha256(a.previous.read_bytes()).hexdigest()
    with a.output.open('x') as f:
        json.dump(report,f,ensure_ascii=False,indent=2,allow_nan=False)
        f.write('\n')
    print(json.dumps(report['counts']))

if __name__ == '__main__':
    main()
