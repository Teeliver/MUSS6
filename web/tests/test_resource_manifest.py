import importlib.util
import tempfile
import unittest
from pathlib import Path
spec = importlib.util.spec_from_file_location('manifest', Path(__file__).parents[1] / 'tools/resource_manifest.py')
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)


class ManifestTests(unittest.TestCase):
    def test_exact_case_hash_and_determinism(self):
        with tempfile.TemporaryDirectory() as folder:
            data = Path(folder)
            (data / 'Item').mkdir()
            (data / 'Item/wing08.bmd').write_bytes(b'fixture')
            audit = {'entries': [{'path': 'Item/Wing08.bmd'}]}
            first = manifest.generate(data, audit)
            self.assertEqual(first, manifest.generate(data, audit))
            self.assertEqual(first['entries'][0]['path'], 'Item/wing08.bmd')
            self.assertEqual(first['entries'][0]['size'], 7)
            (data / 'Item/wing08.bmd').write_bytes(b'changed')
            self.assertNotEqual(first['version'], manifest.generate(data, audit)['version'])

    def test_missing_unsafe_and_symlink_escape(self):
        with tempfile.TemporaryDirectory() as folder, tempfile.TemporaryDirectory() as outside:
            data = Path(folder)
            (data / 'Item').mkdir()
            target = Path(outside) / 'bad.bmd'
            target.write_bytes(b'outside')
            (data / 'Item/bad.bmd').symlink_to(target)
            for path in ['Item/missing.bmd', '../bad.bmd', 'Item/bad.bmd']:
                with self.subTest(path=path), self.assertRaises(ValueError):
                    manifest.generate(data, {'entries': [{'path': path}]})
