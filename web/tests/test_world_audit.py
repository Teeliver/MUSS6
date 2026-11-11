import json
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from summarize_world_audit import category, summarize

BINARY = Path(__file__).resolve().parents[1] / '.local-baselines/2026-10-23-m00-003/world-audit'

def encrypt(data):
    key = bytes.fromhex('D1 73 52 F6 D2 9A CB 27 3E AF 59 31 37 B3 E7 A2')
    previous = 0x5e
    result = bytearray()
    for i, value in enumerate(data):
        encoded = ((value + previous) & 255) ^ key[i % 16]
        result.append(encoded)
        previous = (encoded + 0x3d) & 255
    return result

@unittest.skipUnless(BINARY.is_file(), 'Build world-audit first')
class WorldAuditTests(unittest.TestCase):
    def test_unsupported_missing_model_and_bad_records_are_distinct(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            for world in (1, 2, 4):
                (root / f'World{world}').mkdir()
            record = struct.pack('<h7f', 80, 100, 100, 0, 0, 0, 0, 1)
            (root / 'World1/EncTerrain1.obj').write_bytes(encrypt(bytes((0, 1)) + struct.pack('<H', 1) + record))
            (root / 'World4/EncTerrain4.obj').write_bytes(encrypt(bytes((0, 4)) + struct.pack('<H', 7) + record))
            out = subprocess.run([str(BINARY), folder], text=True, capture_output=True, check=True)
            data = json.loads(out.stdout)
            worlds = {r['world']: r for r in data['worlds']}
            self.assertEqual(worlds[2]['status'], 'unsupported_by_native_parser')
            self.assertEqual(worlds[1]['objects']['dependencies'][0]['model'], 'Object1/Bridge01.bmd')
            self.assertEqual(worlds[1]['objects']['dependencies'][0]['status'], 'missing')
            self.assertEqual(worlds[4]['objects']['status'], 'parser_error')
            self.assertEqual(worlds[1]['terrain']['status'], 'parser_error')
            self.assertEqual(len(data['tables']), 9)
            self.assertTrue(all(r['status'] == 'parser_error' for r in data['tables']))

class ClassificationTests(unittest.TestCase):
    def test_path_classification_never_marks_unknown_tables_decoded(self):
        self.assertEqual(category('World34/minimap.bmd'), 'minimap_by_path')
        self.assertEqual(category('Local/Eng/Item_eng.bmd'), 'supported_local_table')
        self.assertEqual(category('Item/Itemtest.bmd'), 'unclassified_non_model')
        report = summarize({'worlds': [], 'tables': []}, [dict(path='Local/Mix.bmd', kind='non_model_bmd')])
        self.assertEqual(report['non_model_bmd'][0]['status'], 'not_decoded')
