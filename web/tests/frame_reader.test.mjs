import test from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

test('portable C++ framer: fragmentation, coalescing, bounds, EOF and ownership', async () => {
  const dir = await mkdtemp(path.join(tmpdir(), 'mu-frame-test-'));
  const root = fileURLToPath(new URL('../../', import.meta.url));
  const binary = path.join(dir, 'contract');
  try {
    execFileSync('clang++', ['-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic',
      '-I', path.join(root, 'client-core/include'), path.join(root, 'web/tests/frame_reader_contract.cpp'), '-o', binary]);
    assert.match(execFileSync(binary, { encoding: 'utf8' }), /contract passed/);
  } finally { await rm(dir, { recursive: true, force: true }); }
});
