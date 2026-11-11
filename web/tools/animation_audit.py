"""Offline numeric animation audit for BMD10/12 using assets.cpp layout.
Mesh blocks are bounded/skipped; this supplements rather than replaces mesh audit.
"""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import struct
from world_structure import check, decrypt


class Reader:
    def __init__(self, data):
        self.data = memoryview(data)
        self.offset = 0
        self.floats = 0

    def take(self, size):
        check(0 <= size <= len(self.data)-self.offset, f'truncated at payload offset {self.offset}')
        start = self.offset
        self.offset += size
        return self.data[start:self.offset]

    def small(self):
        return struct.unpack('<h',self.take(2))[0]

    def byte(self):
        return self.take(1)[0]

    def numbers(self, count, context):
        start = self.offset
        for i,(value,) in enumerate(struct.iter_unpack('<f', self.take(count*4))):
            check(math.isfinite(value) and abs(value)<1e7,
                  f'{context}: invalid float {value} at payload offset {start+i*4}')
        self.floats += count


def inspect_model(data):
    check(len(data)>=42 and data[:3]==b'BMD','invalid model signature/size')
    version=data[3]
    check(version in (10,12),f'unsupported model version {version}')
    if version==12:
        check(struct.unpack_from('<I',data,4)[0]==len(data)-8,'encrypted size mismatch')
        payload=decrypt(data[8:])
    else:
        payload=data[4:]
    r=Reader(payload)
    r.take(32)
    meshes,bones,actions=r.small(),r.small(),r.small()
    check(0<=meshes<=128 and 0<=bones<=256 and 0<=actions<=512,'invalid model counts')
    for mesh in range(meshes):
        vertices,normals,uv,faces=r.small(),r.small(),r.small(),r.small()
        r.small()
        check(0<=vertices<=30000 and normals>=0 and uv>=0 and faces>=0,'invalid mesh counts')
        r.take(vertices*16+normals*20+uv*8+faces*64+32)
    keys=[]
    total=0
    locked=0
    for action in range(actions):
        count,lock=r.small(),r.byte()
        check(0<count<=4096 and lock<=1,'invalid action header')
        total+=count
        check(total<=100000,'animation key count exceeds bound')
        keys.append(count)
        if lock:
            locked+=1
            r.numbers(count*3,f'locked position action={action}')
    parents=[-1]*bones
    active=0
    for bone in range(bones):
        dummy=r.byte()
        check(dummy<=1,'invalid bone flag')
        if dummy:
            continue
        active+=1
        r.take(32)
        parent=r.small()
        check(-1<=parent<bones and parent!=bone,f'invalid parent bone={bone}: {parent}')
        parents[bone]=parent
        for action,count in enumerate(keys):
            r.numbers(count*3,f'position bone={bone} action={action}')
            r.numbers(count*3,f'rotation bone={bone} action={action}')
    for bone in range(bones):
        seen=set()
        node=bone
        while node>=0:
            check(node not in seen,f'cyclic skeleton bone={bone}')
            seen.add(node)
            node=parents[node]
    check(r.offset==len(payload),'trailing BMD bytes')
    return dict(version=version,meshes=meshes,bones=bones,active_bones=active,actions=actions,
                action_keys=keys,locked_actions=locked,animation_floats=r.floats)


def audit(root):
    root=root.resolve(strict=True)
    rows=[]
    for path in sorted(root.rglob('*')):
        if path.suffix.lower()!='.bmd' or path.is_symlink() or not path.is_file():
            continue
        # Do not follow a symlinked parent outside Data.
        if not path.resolve().is_relative_to(root):
            continue
        with path.open('rb') as f:
            if f.read(3)!=b'BMD':
                continue
        row=dict(path=path.relative_to(root).as_posix())
        rows.append(row)
        try:
            check(0<path.stat().st_size<=64*1024*1024,'model exceeds file bound')
            data=path.read_bytes()
            row['sha256']=hashlib.sha256(data).hexdigest()
            row['layout']=inspect_model(data)
            row['status']='animation_numeric_pass'
        except (ValueError,OSError,struct.error) as e:
            row['status']='animation_audit_error'
            row['error']=str(e)
    return dict(models=rows,counts=dict(Counter(r['status'] for r in rows)),
                limits=['Mesh blocks skipped after bounded counts; use W0002 for mesh/index/texture checks.',
                        'Finite bounded floats do not prove correct animation, timing, interpolation or Windows parity.',
                        'Only BMD versions 10 and 12 supported; no production parser changed.'])


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--data',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    report=audit(a.data.resolve(strict=True))
    with a.output.open('x') as f:
        json.dump(report,f,ensure_ascii=False,indent=2,allow_nan=False)
        f.write('\n')
    print(json.dumps(report['counts']))

if __name__=='__main__':
    main()
