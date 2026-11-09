import ctypes
import re
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class ProgramTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.temp_dir = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp_dir.cleanup)

        build_dir = Path(cls.temp_dir.name)
        cls.executable = build_dir / "code"
        cls.library_path = build_dir / "libback.so"

        # Компиляция основной программы
        subprocess.run(
            [
                "gcc", "-std=c11", "-Wall", "-Wextra", "-O2",
                "-I", str(ROOT),
                str(ROOT / "main.c"),
                str(ROOT / "back.c"),
                str(ROOT / "front.c"),
                "-o", str(cls.executable)
            ],
            check=True,
            capture_output=True,
            text=True
        )

        # Компиляция библиотеки для тестирования функций
        subprocess.run(
            [
                "gcc", "-std=c11", "-Wall", "-Wextra",
                "-fPIC", "-shared",
                "-I", str(ROOT),
                str(ROOT / "back.c"),
                "-o", str(cls.library_path)
            ],
            check=True,
            capture_output=True,
            text=True
        )

        cls.lib = ctypes.CDLL(str(cls.library_path))

        cls.lib.ip_to_uint.argtypes = [ctypes.c_char_p]
        cls.lib.ip_to_uint.restype = ctypes.c_uint32

        cls.lib.parse_mask.argtypes = [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_uint32)
        ]
        cls.lib.parse_mask.restype = ctypes.c_int

        cls.lib.generate_ip.argtypes = []
        cls.lib.generate_ip.restype = ctypes.c_uint32

    def run_program(self, *args):
        return subprocess.run(
            [str(self.executable), *args],
            capture_output=True,
            text=True,
            timeout=10
        )

    # Тест 1. Преобразование IP-адреса
    def test_ip_to_uint(self):
        examples = {
            "0.0.0.0": 0x00000000,
            "192.168.1.1": 0xC0A80101,
            "10.0.0.1": 0x0A000001,
            "255.255.255.255": 0xFFFFFFFF
        }

        for address, expected in examples.items():
            with self.subTest(ip=address):
                self.assertEqual(
                    self.lib.ip_to_uint(address.encode()),
                    expected
                )

    # Тест 2. Маски в формате /N
    def test_prefix_masks(self):
        examples = {
            "/0": 0x00000000,
            "/8": 0xFF000000,
            "/16": 0xFFFF0000,
            "/24": 0xFFFFFF00,
            "/32": 0xFFFFFFFF
        }

        for prefix, expected in examples.items():
            with self.subTest(mask=prefix):
                result = ctypes.c_uint32()

                self.assertEqual(
                    self.lib.parse_mask(prefix.encode(),ctypes.byref(result)),
                    1
                )
                self.assertEqual(result.value, expected)

    # Тест 3. Маски в десятичном формате
    def test_dotted_masks(self):
        examples = {
            "0.0.0.0": 0,
            "255.0.0.0": 0xFF000000,
            "255.255.255.0": 0xFFFFFF00,
            "255.255.255.255": 0xFFFFFFFF
        }

        for mask, expected in examples.items():
            with self.subTest(mask=mask):
                result = ctypes.c_uint32()

                self.assertEqual(
                    self.lib.parse_mask(
                        mask.encode(),
                        ctypes.byref(result)
                    ),
                    1
                )
                self.assertEqual(result.value, expected)

    # Тест 4. Некорректные маски
    def test_invalid_masks(self):
        masks = [
            "", "/", "/-1", "/33", "/abc", "/24x",
            "255.0.255.0", "255.255.255.1",
            "256.0.0.0", "not-a-mask"
        ]

        for mask in masks:
            with self.subTest(mask=mask):
                result = ctypes.c_uint32(0)

                self.assertEqual(
                    self.lib.parse_mask(
                        mask.encode(),
                        ctypes.byref(result)
                    ),
                    0
                )

    # Тест 5. Отсутствуют аргументы
    def test_no_arguments(self):
        result = self.run_program()

        self.assertEqual(result.returncode, 1)
        self.assertIn("Использование:", result.stdout)

    # Тест 6. Неправильное количество аргументов
    def test_wrong_number_of_arguments(self):
        result = self.run_program("192.168.1.1", "/24")

        self.assertEqual(result.returncode, 1)
        self.assertIn("Использование:", result.stdout)

    # Тест 7. Некорректное число пакетов
    def test_nonpositive_packet_count(self):
        for count in ("0", "-5", "abc"):
            with self.subTest(n=count):
                result = self.run_program(
                    "192.168.1.1", "/24", count
                )

                self.assertEqual(result.returncode, 1)
                self.assertIn(
                    "N должно быть положительным",
                    result.stdout
                )

    # Тест 8. Некорректная маска в основной программе
    def test_invalid_mask_in_main(self):
        result = self.run_program(
            "192.168.1.1", "/33", "5"
        )

        self.assertEqual(result.returncode, 1)
        self.assertIn(
            "Некорректная маска подсети",
            result.stdout
        )

    # Тест 9. Все пакеты принадлежат подсети /0
    def test_all_packets_in_own_subnet_with_zero_mask(self):
        result = self.run_program(
            "192.168.1.1", "/0", "5"
        )

        self.assertEqual(result.returncode, 0)

        packets = re.findall(
            r"^\d+\. \d+\.\d+\.\d+\.\d+ -> своя подсеть$",
            result.stdout,
            re.MULTILINE
        )

        self.assertEqual(len(packets), 5)
        self.assertIn(
            "Своя подсеть: 5 (100.00%)",
            result.stdout
        )
        self.assertIn(
            "Другие сети: 0 (0.00%)",
            result.stdout
        )

    # Тест 10. Десятичная маска в основной программе
    def test_dotted_mask_in_main(self):
        result = self.run_program(
            "192.168.1.1", "0.0.0.0", "3"
        )

        self.assertEqual(result.returncode, 0)
        self.assertIn(
            "Своя подсеть: 3 (100.00%)",
            result.stdout
        )


if __name__ == "__main__":
    unittest.main()