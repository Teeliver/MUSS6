#!/usr/bin/env python3
"""Read-only inventory. No database access, decoder or server startup.

Paths and hashes are evidence of bytes on disk, not evidence of valid assets or
visual parity. Private configuration is never included in the source archive.
"""
import argparse
import collections
import hashlib
import json
import os
from pathlib import Path
import tarfile
import tempfile

SOURCE_SUFFIXES = {'.cpp', '.c', '.h', '.hpp', '.mm', '.m', '.cmake', '.py'}
EXCLUDED = {'.git', 'node_modules', '__pycache__', '.local-baselines', 'runtime', 'build'}


def digest(path):
    before = path.stat()
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(chunk)
    after = path.stat()
    if (before.st_size, before.st_mtime_ns) != (after.st_size, after.st_mtime_ns):
        raise RuntimeError('File changed during inventory: ' + str(path))
    return value.hexdigest(), after.st_size


def inventory(root, source=False):
    root = Path(root).resolve(strict=True)
    if not root.is_dir():
        raise ValueError('Inventory root must be a directory')
    files, links, errors = [], [], []
    def walk_error(error):
        errors.append({'path': str(error.filename), 'error': type(error).__name__})
    for directory, dirs, names in os.walk(root, followlinks=False, onerror=walk_error):
        dirs.sort()
        for name in list(dirs):
            path = Path(directory) / name
            if path.is_symlink():
                links.append(path.relative_to(root).as_posix())
                dirs.remove(name)
            elif source and name in EXCLUDED:
                dirs.remove(name)
        for name in sorted(names):
            path = Path(directory) / name
            relative = path.relative_to(root).as_posix()
            if path.is_symlink():
                links.append(relative)
                continue
            if source and path.suffix.lower() not in SOURCE_SUFFIXES and name != 'CMakeLists.txt':
                continue
            try:
                sha, size = digest(path)
                files.append({'path': relative, 'bytes': size, 'sha256': sha})
            except (OSError, RuntimeError) as error:
                errors.append({'path': relative, 'error': type(error).__name__})
    files.sort(key=lambda row: row['path'])
    by_case = collections.defaultdict(list)
    extensions = collections.Counter()
    for row in files:
        by_case[row['path'].casefold()].append(row['path'])
        extensions[Path(row['path']).suffix.lower() or '(none)'] += 1
    fingerprint = hashlib.sha256(json.dumps(files, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    return {'schema': 1, 'file_count': len(files), 'total_bytes': sum(f['bytes'] for f in files),
            'manifest_sha256': fingerprint, 'extensions': dict(sorted(extensions.items())),
            'case_collisions': [v for _, v in sorted(by_case.items()) if len(v) > 1],
            'symlinks_skipped': sorted(links), 'errors': errors, 'files': files,
            'dependency_status': 'not-decoded', 'validation_status': 'hash-only'}


def write_json(path, value):
    with tempfile.NamedTemporaryFile(mode='w', encoding='utf8', dir=path.parent, delete=False) as stream:
        temporary = Path(stream.name)
        json.dump(value, stream, ensure_ascii=False, sort_keys=True, indent=2)
        stream.write('\n')
    os.replace(temporary, path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data', type=Path, required=True)
    parser.add_argument('--source', action='append', default=[], metavar='NAME=PATH')
    parser.add_argument('--config', action='append', default=[], metavar='NAME=PATH',
                        help='Fingerprint configuration only; never archive its contents')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    data, output = args.data.resolve(strict=True), args.output.resolve()
    if output == data or data in output.parents:
        parser.error('Output must be outside original Data')
    if output.exists():
        parser.error('Output already exists; use a new immutable baseline directory')
    sources = {}
    for entry in args.source:
        name, separator, path = entry.partition('=')
        if not separator or not name or not name.replace('-', '').replace('_', '').isalnum() or name in sources:
            parser.error('Source requires a unique safe NAME=PATH')
        sources[name] = Path(path).resolve(strict=True)
    configs = {}
    for entry in args.config:
        name, separator, path = entry.partition('=')
        if not separator or not name or not name.replace('-', '').replace('_', '').isalnum() or name in configs or name in sources:
            parser.error('Config requires a unique safe NAME=PATH')
        configs[name] = Path(path).resolve(strict=True)
    output.mkdir(mode=0o700, parents=True)
    print('Hashing Data...', flush=True)
    assets = inventory(data)
    write_json(output / 'data.manifest.json', assets)
    manifests = {}
    for name, root in sorted(sources.items()):
        print('Hashing source: ' + name, flush=True)
        manifests[name] = inventory(root, source=True)
        write_json(output / (name + '.manifest.json'), manifests[name])
    config_manifests = {}
    for name, root in sorted(configs.items()):
        config_manifests[name] = inventory(root)
        write_json(output / (name + '.manifest.json'), config_manifests[name])
    # Only source files, never .ini/.dat/credential/runtime content.
    archive_path = output / 'source.tar.gz'
    with tarfile.open(archive_path, 'w:gz') as archive:
        for name, manifest in sorted(manifests.items()):
            for row in manifest['files']:
                path = sources[name] / row['path']
                if digest(path)[0] != row['sha256']:
                    raise RuntimeError('Source changed before archival: ' + row['path'])
                archive.add(path, arcname=name + '/' + row['path'], recursive=False)
    # Verify actual archived bytes, not just the source before opening tar input.
    expected = {name + '/' + row['path']: row['sha256']
                for name, manifest in manifests.items() for row in manifest['files']}
    with tarfile.open(archive_path, 'r:gz') as archive:
        for member in archive:
            if member.name not in expected or not member.isfile():
                raise RuntimeError('Unexpected archive entry')
            with archive.extractfile(member) as stream:
                value = hashlib.sha256()
                for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                    value.update(chunk)
            if value.hexdigest() != expected.pop(member.name):
                raise RuntimeError('Archive content differs from source manifest')
    if expected:
        raise RuntimeError('Archive is missing source files')
    errors = len(assets['errors']) + sum(len(v['errors']) for v in manifests.values()) + sum(len(v['errors']) for v in config_manifests.values())
    summary = {'schema': 1, 'data_files': assets['file_count'], 'data_bytes': assets['total_bytes'],
               'data_manifest_sha256': assets['manifest_sha256'],
               'source_files': {name: m['file_count'] for name, m in manifests.items()},
               'config_files_hash_only': {name: m['file_count'] for name, m in config_manifests.items()},
               'archive_sha256': digest(archive_path)[0], 'read_errors': errors,
               'status': 'incomplete' if errors else 'hash-baseline-complete',
               'not_verified': ['asset dependencies', 'decoding', 'Windows EX603 parity',
                                'DB backup', 'browser execution', 'performance']}
    write_json(output / 'summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False, indent=2), flush=True)
    return 1 if errors else 0


if __name__ == '__main__':
    raise SystemExit(main())
