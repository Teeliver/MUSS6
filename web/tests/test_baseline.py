import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('baseline', Path(__file__).parents[1] / 'tools/baseline.py')
baseline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(baseline)


class BaselineTests(unittest.TestCase):
    def test_content_changes_fingerprint_without_changing_source(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'Player.bmd'
            path.write_bytes(b'original')
            first = baseline.inventory(directory)
            self.assertEqual(path.read_bytes(), b'original')
            self.assertEqual(first, baseline.inventory(directory))
            path.write_bytes(b'changed!')
            self.assertNotEqual(first['manifest_sha256'], baseline.inventory(directory)['manifest_sha256'])

    def test_source_archive_selection_excludes_private_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ['player.cpp', 'mysql.ini', 'credentials.txt', 'CMakeLists.txt']:
                (root / name).write_text('fixture')
            (root / 'runtime').mkdir()
            (root / 'runtime/secret.cpp').write_text('fixture')
            result = baseline.inventory(root, source=True)
            self.assertEqual([r['path'] for r in result['files']], ['CMakeLists.txt', 'player.cpp'])

    def test_does_not_follow_symlink_outside_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'data').mkdir()
            (root / 'outside.txt').write_text('fixture')
            (root / 'data/link').symlink_to(root / 'outside.txt')
            result = baseline.inventory(root / 'data')
            self.assertEqual(result['files'], [])
            self.assertEqual(result['symlinks_skipped'], ['link'])


if __name__ == '__main__':
    unittest.main()
