from pathlib import Path
import math
import struct
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1] / 'tools'))
from animation_audit import inspect_model, audit
from test_world_audit import encrypt


def model(second=0., locked=False):
    p=bytes(32)+struct.pack('<hhh',0,1,2)
    for a in range(2):
        p+=struct.pack('<hB',1,int(locked))
        if locked:
            p+=struct.pack('<3f',0,0,0)
    p+=b'\x00'+bytes(32)+struct.pack('<h',-1)
    p+=struct.pack('<12f',0,0,0,0,0,0,second,0,0,0,0,0)
    return b'BMD\x0a'+p


class AnimationTests(unittest.TestCase):
    def test_all_actions_and_encrypted_layout(self):
        data=model(locked=True)
        self.assertEqual(inspect_model(data)['animation_floats'],18)
        encrypted=encrypt(data[4:])
        result=inspect_model(b'BMD\x0c'+struct.pack('<I',len(encrypted))+encrypted)
        self.assertEqual(result['action_keys'],[1,1])
        self.assertEqual(result['version'],12)

    def test_nan_in_action_not_retained_by_old_audit(self):
        with self.assertRaisesRegex(ValueError,'bone=0 action=1'):
            inspect_model(model(math.nan))
        with self.assertRaisesRegex(ValueError,'invalid float'):
            inspect_model(model(1e8))

    def test_locked_root_nan_and_truncation(self):
        b=bytearray(model(locked=True))
        struct.pack_into('<f',b,45,math.nan)
        with self.assertRaisesRegex(ValueError,'locked position'):
            inspect_model(b)
        with self.assertRaises(ValueError):inspect_model(model()[:-1])
        with self.assertRaises(ValueError):inspect_model(model()+b'bad')

    def test_cycle_even_without_mesh_and_table_not_counted(self):
        p=bytes(32)+struct.pack('<hhh',0,2,0)
        for parent in (1,0):p+=b'\x00'+bytes(32)+struct.pack('<h',parent)
        with self.assertRaisesRegex(ValueError,'cyclic'):
            inspect_model(b'BMD\x0a'+p)
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            (root/'valid.bmd').write_bytes(model())
            (root/'table.bmd').write_bytes(b'table')
            self.assertEqual(audit(root)['counts'],{'animation_numeric_pass':1})
