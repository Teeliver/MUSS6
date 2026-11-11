import math
from pathlib import Path
import struct
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from world_structure import audit, object_layout, mapping_layout, attribute_layout, height_layout
from test_world_audit import encrypt


class StructureTests(unittest.TestCase):
    def test_all_world_schema_does_not_enable_renderer(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            d = root / 'World2'
            d.mkdir()
            (d / 'EncTerrain2.obj').write_bytes(encrypt(bytes((0,2,0,0))))
            (root / 'World3').symlink_to(d, target_is_directory=True)
            rows = audit(root)['worlds']
            self.assertEqual(len(rows), 1)
            self.assertFalse(rows[0]['native_world_supported'])
            self.assertEqual(rows[0]['files'][0]['status'], 'schema_parsed')
            self.assertEqual(rows[0]['files'][1]['status'], 'schema_error')

    def test_object_nan_truncation_and_count_mismatch(self):
        header = bytes((0,2,7,0))
        record = struct.pack('<h7f', 1, 100,100,0,0,0,0,1)
        result = object_layout(encrypt(header + record), 2)
        self.assertEqual(result['declared_delta'], 6)
        self.assertFalse(result['native_count_compatible'])
        with self.assertRaises(ValueError):
            object_layout(encrypt(header + record[:-1]), 2)
        with self.assertRaises(ValueError):
            object_layout(encrypt(header + struct.pack('<h7f', 1, math.nan,100,0,0,0,0,1)), 2)

    def test_mapping_and_attribute_layouts(self):
        data = bytes((0,2)) + bytes(65536) + bytes([255])*65536 + bytes(65536)
        self.assertFalse(mapping_layout(encrypt(data), 2)['tile_mapping_verified'])
        with self.assertRaises(ValueError):
            mapping_layout(encrypt(data), 3)
        for bits in (8,16):
            raw = bytes((0,2,255,255)) + bytes(65536*(bits//8))
            mask = bytes.fromhex('FC CF AB')
            encoded = encrypt(bytes(v ^ mask[i%3] for i,v in enumerate(raw)))
            self.assertEqual(attribute_layout(encoded,2)['bits'], bits)

    def test_height_reports_legacy_padding_without_claiming_alignment(self):
        b = bytearray(1082+65536)
        b[4:6] = b'BM'
        struct.pack_into('<I',b,14,1078)
        struct.pack_into('<IiiHHI',b,18,40,256,256,1,8,0)
        r = height_layout(b,2)
        self.assertEqual(r['native_tail_padding'], 2)
        self.assertFalse(r['visual_alignment_verified'])
        with self.assertRaises(ValueError):
            height_layout(b[:-1],2)
