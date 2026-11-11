import importlib.util
import unittest
from pathlib import Path
spec = importlib.util.spec_from_file_location('skill_class', Path(__file__).parents[1] / 'tools/skill_class_audit.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
class SkillClassTests(unittest.TestCase):
    def test_columns_and_changeup_rule(self):
        row = '4 "Fire Ball" ' + ' '.join(['0'] * 13 + ['1', '0', '0', '1', '0', '2', '0'])
        skill = module.parse('// header\n' + row + '\nend')[0]
        self.assertTrue(module.allowed(skill, 0, 0))
        self.assertFalse(module.allowed(skill, 1, 2))
        self.assertFalse(module.allowed(skill, 5, 0))
        self.assertTrue(module.allowed(skill, 5, 1))
        with self.assertRaises(ValueError):
            module.parse(row + '\n' + row)
        with self.assertRaises(ValueError):
            module.parse('4 "incomplete" 1')
if __name__ == '__main__':
    unittest.main()
