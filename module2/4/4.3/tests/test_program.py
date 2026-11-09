import ctypes
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAX_STR = 100


class SocialMedia(ctypes.Structure):
    _fields_ = [(name, ctypes.c_char * MAX_STR)
                for name in ('telegram', 'vk', 'other')]


class Contact(ctypes.Structure):
    _fields_ = [
        ('surname', ctypes.c_char * MAX_STR),
        ('name', ctypes.c_char * MAX_STR),
        ('patronymic', ctypes.c_char * MAX_STR),
        ('workplace', ctypes.c_char * MAX_STR),
        ('position', ctypes.c_char * MAX_STR),
        ('phones', (ctypes.c_char * MAX_STR) * 3),
        ('emails', (ctypes.c_char * MAX_STR) * 3),
        ('social', SocialMedia),
    ]


class ContactNode(ctypes.Structure):
    pass


ContactNode._fields_ = [
    ('contact', Contact),
    ('order', ctypes.c_size_t),
    ('height', ctypes.c_int),
    ('left', ctypes.POINTER(ContactNode)),
    ('right', ctypes.POINTER(ContactNode)),
]


class PhoneBook(ctypes.Structure):
    _fields_ = [
        ('root', ctypes.POINTER(ContactNode)),
        ('count', ctypes.c_size_t),
        ('next_order', ctypes.c_size_t),
    ]


def contact(surname, name='Иван', patronymic='', phone='', email=''):
    item = Contact()
    item.surname = surname.encode('utf-8')
    item.name = name.encode('utf-8')
    item.patronymic = patronymic.encode('utf-8')
    if phone:
        item.phones[0].value = phone.encode('utf-8')
    if email:
        item.emails[0].value = email.encode('utf-8')
    return item


def cli_contact(surname, name, phone=''):
    return [surname, name, '', '', '', phone, '', '', '', '', '', '', '', '']


class PhoneBookTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        temporary = tempfile.TemporaryDirectory()
        cls.addClassCleanup(temporary.cleanup)
        output = Path(temporary.name)
        cls.executable = output / 'phonebook'
        library = output / 'libphonebook.so'

        subprocess.run([
            'gcc', '-std=c11', '-Wall', '-Wextra', '-O2',
            '-I', str(ROOT), str(ROOT / 'main.c'),
            str(ROOT / 'back.c'), str(ROOT / 'front.c'),
            '-o', str(cls.executable),
        ], check=True, capture_output=True, text=True)
        subprocess.run([
            'gcc', '-std=c11', '-Wall', '-Wextra', '-fPIC', '-shared',
            '-I', str(ROOT), str(ROOT / 'back.c'), '-o', str(library),
        ], check=True, capture_output=True, text=True)

        cls.lib = ctypes.CDLL(str(library))
        cls.lib.add_contact.argtypes = [ctypes.POINTER(PhoneBook), ctypes.POINTER(Contact)]
        cls.lib.add_contact.restype = ctypes.c_int
        cls.lib.get_contact.argtypes = [ctypes.POINTER(PhoneBook), ctypes.c_size_t]
        cls.lib.get_contact.restype = ctypes.POINTER(Contact)
        cls.lib.edit_contact.argtypes = [ctypes.POINTER(PhoneBook), ctypes.c_size_t,
                                         ctypes.POINTER(Contact)]
        cls.lib.edit_contact.restype = ctypes.c_int
        cls.lib.delete_contact.argtypes = [ctypes.POINTER(PhoneBook), ctypes.c_size_t]
        cls.lib.delete_contact.restype = None
        cls.lib.free_contacts.argtypes = [ctypes.POINTER(ContactNode)]
        cls.lib.free_contacts.restype = None

    def setUp(self):
        self.book = PhoneBook()

    def tearDown(self):
        self.lib.free_contacts(self.book.root)

    def add(self, surname, name='Мишаня', patronymic='', phone='', email=''):
        item = contact(surname, name, patronymic, phone, email)
        self.assertEqual(self.lib.add_contact(ctypes.byref(self.book), ctypes.byref(item)), 1)

    def names(self):
        return [self.lib.get_contact(ctypes.byref(self.book), i).contents.surname.decode('utf-8')
                for i in range(self.book.count)]

    def run_menu(self, lines):
        return subprocess.run(
            [str(self.executable)], input='\n'.join(lines) + '\n',
            capture_output=True, text=True, timeout=5,
            env={**os.environ, 'LC_ALL': 'C.UTF-8'},
        )

    def test_empty_book(self):
        self.assertEqual(self.book.count, 0)
        self.assertFalse(bool(self.lib.get_contact(ctypes.byref(self.book), 0)))

    def test_add_contact(self):
        self.add('Шалопаев', 'Иван', phone='123456', email='aboba228@gmail.com')
        self.assertEqual(self.book.count, 1)
        saved = self.lib.get_contact(ctypes.byref(self.book), 0).contents
        self.assertEqual(saved.surname, 'Шалопаев'.encode('utf-8'))
        self.assertEqual(saved.phones[0].value, b'123456')
        self.assertEqual(saved.emails[0].value, 'aboba228@gmail.com'.encode('utf-8'))

    def test_sort_by_surname(self):
        for surname in ('Пескарь', 'Савельев', 'Марков'):
            self.add(surname)
        self.assertEqual(self.names(), ['Марков', 'Пескарь', 'Савельев'])

    def test_sort_by_name_and_patronymic(self):
        for name, patronymic in [('Борян', ''), ('Анна', 'Яяяяяяяя'), ('Анна', 'Аааааааа')]:
            self.add('Шалопаев', name, patronymic)
        result = [self.lib.get_contact(ctypes.byref(self.book), i).contents
                  for i in range(3)]
        self.assertEqual([(x.name, x.patronymic) for x in result],
                         [('Анна'.encode('utf-8'), 'Аааааааа'.encode('utf-8')), ('Анна'.encode('utf-8'), 'Яяяяяяяя'.encode('utf-8')), ('Борян'.encode('utf-8'), b'')])

    def test_duplicate_names_preserve_insertion_order(self):
        for phone in ('111', '222', '333'):
            self.add('Шалопаев', phone=phone)
        self.assertEqual([self.lib.get_contact(ctypes.byref(self.book), i)
                          .contents.phones[0].value for i in range(3)],
                         [b'111', b'222', b'333'])

    def test_edit_contact_changes_sort_position(self):
        self.add('Иванов')
        self.add('Петров')
        replacement = contact('Сидоров', 'Александр')
        self.assertEqual(self.lib.edit_contact(ctypes.byref(self.book), 0,
                                               ctypes.byref(replacement)), 1)
        self.assertEqual(self.names(), ['Петров', 'Сидоров'])
        self.assertEqual(self.book.count, 2)

    def test_delete_only_contact(self):
        self.add('Иванов')
        self.lib.delete_contact(ctypes.byref(self.book), 0)
        self.assertEqual(self.book.count, 0)
        self.assertFalse(bool(self.book.root))

    def test_delete_middle_contact(self):
        for surname in ('Иванов', 'Петров', 'Сидоров'):
            self.add(surname)
        self.lib.delete_contact(ctypes.byref(self.book), 1)
        self.assertEqual(self.names(), ['Иванов', 'Сидоров'])
        self.assertEqual(self.book.count, 2)

    def test_avl_balancing_after_many_insertions(self):
        for i in range(100):
            self.add(f'Контакт{i:03d}')
        self.assertEqual(self.book.count, 100)
        self.assertEqual(self.names(), [f'Контакт{i:03d}' for i in range(100)])

        def check_node(ptr):
            if not ptr:
                return 0
            node = ptr.contents
            left = check_node(node.left)
            right = check_node(node.right)
            self.assertLessEqual(abs(left - right), 1)
            self.assertEqual(node.height, max(left, right) + 1)
            return node.height

        self.assertLessEqual(check_node(self.book.root), 8)

    def test_cli_exit(self):
        result = self.run_menu(['0'])
        self.assertEqual(result.returncode, 0)
        self.assertIn('ТЕЛЕФОННАЯ КНИГА', result.stdout)

    def test_cli_empty_list(self):
        result = self.run_menu(['2', '0'])
        self.assertEqual(result.returncode, 0)
        self.assertIn('Телефонная книга пуста.', result.stdout)

    def test_cli_invalid_command(self):
        result = self.run_menu(['9', '0'])
        self.assertEqual(result.returncode, 0)
        self.assertIn('Неверная команда.', result.stdout)

    def test_cli_add_view_delete(self):
        result = self.run_menu(['1', *cli_contact('Иванов', 'Иван', '12345'),
                                '2', '3', '1', '5', '1', '2', '0'])
        self.assertEqual(result.returncode, 0)
        self.assertIn('Контакт добавлен.', result.stdout)
        self.assertIn('Ф.И.О.: Иванов Иван', result.stdout)
        self.assertIn('Телефон 1: 12345', result.stdout)
        self.assertIn('Контакт удалён.', result.stdout)
        self.assertIn('Телефонная книга пуста.', result.stdout)

    def test_cli_invalid_required_name_and_phone(self):
        fields = cli_contact('Иванов', 'Иван', '12345')
        fields.insert(5, '123-45')
        result = self.run_menu(['1', '', *fields, '0'])
        self.assertEqual(result.returncode, 0)
        self.assertIn('поле обязательно', result.stdout)
        self.assertIn('номер должен содержать только цифры', result.stdout)
        self.assertIn('Контакт добавлен.', result.stdout)



if __name__ == '__main__':
    unittest.main()
