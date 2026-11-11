import importlib.util
import tempfile
import unittest
from pathlib import Path
spec = importlib.util.spec_from_file_location('catalog', Path(__file__).parents[1] / 'tools/custom_catalog_audit.py')
catalog = importlib.util.module_from_spec(spec)
spec.loader.exec_module(catalog)


class CatalogTests(unittest.TestCase):
    def test_schema_and_source_directory_rules(self):
        wing = '0 12,200 ' + ' '.join(['0'] * 20) + ' "Wing200"\nend'
        self.assertEqual(catalog.parse_catalog(wing, 'wing', 50)[0]['model'], 'Item/Wing200.bmd')
        rows = catalog.parse_catalog('7,35 1 2 3 "Helm01"\n0,1 4 5 6 "Sword01"\nend', 'item', 2000)
        self.assertEqual([r['model'] for r in rows], ['Player/Helm01.bmd', 'Item/Sword01.bmd'])
        self.assertEqual(catalog.parse_catalog('// comment\nend', 'wing', 50), [])

    def test_invalid_and_duplicate_catalogs_reject(self):
        for text in ['0,1 0 0 0 "../bad"\nend', '0,1 256 0 0 "Good"\nend', '0,512 0 0 0 "Good"\nend', 'end\n1', '0,1 0 0 0 "Good"', '0,1 0 0 0 "Good"\n0,1 0 0 0 "Other"\nend']:
            with self.subTest(text=text), self.assertRaises(ValueError):
                catalog.parse_catalog(text, 'item', 2000)

    def test_asset_case_and_collision_evidence(self):
        with tempfile.TemporaryDirectory() as d:
            data = Path(d)
            (data / 'Item').mkdir()
            (data / 'Item/wing.bmd').write_bytes(b'fixture')
            rows = [dict(index=0, itemIndex=6144, model='Item/Wing.bmd'), dict(index=1, itemIndex=6145, model='Item/Missing.bmd')]
            self.assertEqual([r['assetStatus'] for r in catalog.inspect_assets(rows, data)], ['case-mismatch', 'missing'])
        self.assertEqual([i for i in range(50) if catalog.wing_code(i) in {32, 64, 96, 128, 160, 192}], [15, 31, 47])
