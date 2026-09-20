import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

MODULE_PATH = Path(__file__).resolve().parent / 'docker-entrypoint.py'


class DockerAdapterTests(unittest.TestCase):
    def setUp(self):
        spec = importlib.util.spec_from_file_location('adapter', MODULE_PATH)
        self.adapter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.adapter)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.adapter.BOOK = self.root / 'book'
        self.adapter.OUTPUT = self.root / 'out'
        self.adapter.COMMON = self.root / 'tool'
        ko = self.adapter.BOOK / 'i18n/ko'
        ko.mkdir(parents=True)
        self.adapter.OUTPUT.mkdir()
        (ko / 'build.json').write_text(json.dumps({'profile': 'bgnet', 'lang': 'ko'}))
        (ko / 'toolchain.lock').write_text(json.dumps({'bgbspd_ko': 'a' * 40, 'bgbspd': 'b' * 40}))

    def test_rejects_image_lock_mismatch(self):
        with patch('sys.argv', ['runner']), patch('subprocess.check_output', return_value='c' * 40):
            with self.assertRaisesRegex(ValueError, 'rebuild'):
                self.adapter.main()

    def test_clean_refuses_unowned_output(self):
        sentinel = self.adapter.OUTPUT / 'important.txt'
        sentinel.write_text('keep')
        with patch('sys.argv', ['runner', '--clean']), patch('subprocess.check_output', side_effect=['a' * 40, 'b' * 40]):
            with self.assertRaisesRegex(ValueError, 'not owned'):
                self.adapter.main()
        self.assertEqual(sentinel.read_text(), 'keep')

    def test_published_aliases_and_source_zip(self):
        html = self.adapter.OUTPUT / 'html'
        html.mkdir()
        for name in ['bgnet.html', 'bgnet-wide.html', 'bgnet-split.zip', 'bgnet-split-wide.zip']:
            (html / name).write_bytes(name.encode())
        source = self.adapter.OUTPUT / 'source'
        source.mkdir()
        (source / 'sample.c').write_text('sample')
        (html / 'source').mkdir()
        with patch('sys.argv', ['runner', '--format', 'html']), patch('subprocess.check_output', side_effect=['a' * 40, 'b' * 40]), patch('subprocess.call', return_value=0):
            self.assertEqual(self.adapter.main(), 0)
        self.assertEqual((html / 'index.html').read_bytes(), b'bgnet.html')
        self.assertEqual((html / 'bgnet.zip').read_bytes(), b'bgnet-split.zip')
        with zipfile.ZipFile(source / 'bgnet_source.zip') as z:
            self.assertEqual(z.namelist(), ['bgnet_source/sample.c'])
        self.assertEqual((html / 'source/bgnet_source.zip').read_bytes(), (source / 'bgnet_source.zip').read_bytes())


if __name__ == '__main__':
    unittest.main()
