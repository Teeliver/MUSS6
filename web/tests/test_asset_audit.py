"""Offline integration tests; build the audit CLI once before running these tests."""
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

BINARY = Path(__file__).resolve().parents[1] / '.local-baselines/2026-10-23-m00-002/asset-audit'

@unittest.skipUnless(BINARY.is_file(), 'Build asset-audit first (see ASSET-AUDIT.vi.md)')
class AssetAuditTests(unittest.TestCase):
    def test_classifies_model_table_and_rejects_truncated_model(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            # Valid empty model with one dummy bone; no actions/meshes.
            (root / 'empty.bmd').write_bytes(b'BMD\x0a' + bytes(32) + struct.pack('<hhh', 0, 1, 0) + b'\x01')
            (root / 'table.bmd').write_bytes(b'table data')
            (root / 'missing.bmd').write_bytes(b'BMD\x0a' + bytes(32) + struct.pack('<hhh', 1, 1, 0) + bytes(10) + b'absent.jpg'.ljust(32, b'\x00') + b'\x01')
            (root / 'broken.bmd').write_bytes(b'BMD\x0a' + bytes(37))
            # Explicit invalid parent index must not silently pass validation.
            (root / 'parent.bmd').write_bytes(b'BMD\x0a' + bytes(32) + struct.pack('<hhh', 0, 1, 0) + b'\x00' + bytes(32) + struct.pack('<h', 1))
            (root / 'bad.OZT').write_bytes(b'bad')
            (root / 'outside.bmd').symlink_to('/nonexistent')
            before = {p.name: p.read_bytes() for p in root.iterdir() if not p.is_symlink()}
            result = subprocess.run([str(BINARY), folder], check=True, text=True, capture_output=True)
            rows = {r['path']: r for r in map(json.loads, result.stdout.splitlines())}
            self.assertEqual(rows['empty.bmd']['status'], 'parsed')
            self.assertEqual(rows['table.bmd']['status'], 'not_audited')
            self.assertEqual(rows['missing.bmd']['dependencies'][0]['status'], 'unresolved')
            for name in ('broken.bmd', 'parent.bmd', 'bad.OZT'):
                self.assertEqual(rows[name]['status'], 'parser_error')
            self.assertNotIn('outside.bmd', rows)
            self.assertEqual(before, {p.name: p.read_bytes() for p in root.iterdir() if not p.is_symlink()})

if __name__ == '__main__':
    unittest.main()
