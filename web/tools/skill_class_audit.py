#!/usr/bin/env python3
"""Read-only SkillList class matrix following SkillManager::CheckSkillRequireClass."""
import argparse
import hashlib
import json
import shlex
from pathlib import Path
NAMES = ['DW', 'DK', 'Elf', 'MG', 'DL', 'Summoner', 'RF']

def parse(text):
    skills = []
    seen = set()
    for number, line in enumerate(text.splitlines(), 1):
        line = line.split('//', 1)[0].strip()
        if not line:
            continue
        if line == 'end':
            break
        row = shlex.split(line)
        if len(row) != 22:
            raise ValueError(f'Unsupported SkillList columns at line {number}')
        index = int(row[0])
        requires = list(map(int, row[15:22]))
        if index in seen or index < 0 or any(value < 0 for value in requires):
            raise ValueError(f'Invalid skill identity/class rule at line {number}')
        seen.add(index)
        skills.append({'index': index, 'name': row[1], 'requireClass': requires})
    return skills

def allowed(skill, cls, tier):
    if cls not in range(7) or tier not in range(3):
        raise ValueError('Invalid class/tier')
    required = skill['requireClass'][cls]
    return required != 0 and tier + 1 >= required

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('path', type=Path)
    args = parser.parse_args()
    raw = args.path.read_bytes()
    skills = parse(raw.decode('latin1'))
    fire = next(s for s in skills if s['index'] == 4)
    classes = []
    for cls, name in enumerate(NAMES):
        classes.append({'class': cls, 'name': name, 'skillsByTier': [[s['index'] for s in skills if allowed(s, cls, tier)] for tier in range(3)], 'fireBallAllowedTier0': allowed(fire, cls, 0), 'completePortableSkillSet': False})
    print(json.dumps({'source': str(args.path), 'sha256': hashlib.sha256(raw).hexdigest(), 'rule': 'RequireClass!=0 and ChangeUp+1>=RequireClass; excludes all other skill conditions', 'encoding': 'latin1 byte-preserving audit, not client localization', 'skills': skills, 'classes': classes}, indent=2))
