from pathlib import Path
import hashlib
import json
import math
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1] / 'tools'))
from world_exceptions import inspect_height, inspect_objects, investigate
from test_world_audit import encrypt


def bitmap(bits=24, height=256, colors=0):
    offset = 58 + (colors*4 if bits==8 else 0)
    b = bytearray(offset + 256*256*(bits//8))
    b[4:6] = b'BM'
    struct.pack_into('<I',b,6,len(b)-4)
    struct.pack_into('<I',b,14,offset-4)
    struct.pack_into('<IiiHHI',b,18,40,256,height,1,bits,0)
    struct.pack_into('<I',b,50,colors)
    return b


class ExceptionTests(unittest.TestCase):
    def test_24bit_decodes_rows_without_inventing_mu_height(self):
        b=bitmap()
        b[58:58+768]=b'\xff'*768
        b[-768:]=b'\x01'*768
        result=inspect_height(b)
        expected=b'\x01'*768+bytes(254*768)+b'\xff'*768
        self.assertEqual(result['pixel_sha256'],hashlib.sha256(expected).hexdigest())
        self.assertEqual(result['status'],'bmp_pixels_decoded')
        self.assertFalse(result['usable_height'])
        top=bitmap(height=-256)
        self.assertEqual(inspect_height(top)['status'],'bmp_pixels_decoded')

    def test_palette_truncated_and_zero_are_distinct(self):
        self.assertEqual(inspect_height(bitmap(8, colors=120))['palette_entries'],120)
        self.assertEqual(inspect_height(bitmap()[:-1])['status'],'truncated_bmp_pixels')
        self.assertEqual(inspect_height(bytes(66620))['status'],'all_zero_file')
        with self.assertRaises(ValueError):
            inspect_height(bytes(1084)+bytes([1])*65536)

    def test_all_placement_issues_recorded_and_json_is_finite(self):
        b=bytes((0,3,2,0))+struct.pack('<h7f',1,1,math.nan,0,0,0,0,1)+struct.pack('<h7f',2,1,1,0,0,0,0,-.18)
        result=inspect_objects(encrypt(b),3)
        self.assertEqual(result['affected_records'],2)
        self.assertEqual(result['issues'][1]['byte_offset'],34)
        json.dumps(result,allow_nan=False)

    def test_rejects_input_drift_and_report_path_escape(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            path=root/'height.OZB';path.write_bytes(bytes(66620))
            report={'worlds':[{'world':1,'files':[{'path':'height.OZB','status':'schema_error','sha256':'wrong'}]}]}
            with self.assertRaises(ValueError):investigate(root,report)
            report['worlds'][0]['files'][0]['path']='../outside'
            with self.assertRaises(ValueError):investigate(root,report)
