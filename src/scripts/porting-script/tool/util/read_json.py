# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from tool.util.logger import Logger
LOGGER = Logger().getlogger()

class ReadJsonToDict:
    @staticmethod
    def dict_from_path(filename):
        try:
            with open(filename, 'r') as (f):
                content = f.read()
                dict_content = {}
                dict_content = eval(content)
            return dict_content

        except IOError as io_error:
            try:
                LOGGER.error('Except:%s.', io_error)
            finally:
                io_error = None
                del io_error

        except ValueError as value_error:
            try:
                LOGGER.error('Except:%s.', value_error)
            finally:
                value_error = None
                del value_error

        except IndexError as index_error:
            try:
                LOGGER.error('Except:%s.', index_error)
            finally:
                index_error = None
                del index_error

        except KeyError as key_error:
            try:
                LOGGER.error('Except:%s.', key_error)
            finally:
                key_error = None
                del key_error

        except KeyboardInterrupt as exp:
            try:
                LOGGER.error('Ctrl-C received. Except:%s.', exp)
            finally:
                exp = None
                del exp

if __name__ == '__main__':
    dict_content = ReadJsonToDict.dict_from_path('your work path/config/GCC_8_3/X86_64_COMMANDS_GCC_8_3.json')
    print(dict_content.keys())
    print(dict_content)