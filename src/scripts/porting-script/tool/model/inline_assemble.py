# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

ASSEMBLES_DICT = {
    '__assembles__': {'description': "Found arch specific assembles using '__asm'/'__asm__'/'asm'/'_asm'.",
                      'suggestion':
                      {"arm64": 'Check these assembles to see if it need to port.',
                       "mips64el": 'Check these assembles to see if it need to port.',
                       "x86_64": 'Check these assembles to see if it need to port.',
                       "sw_64": 'Check these assembles to see if it need to port.', 
                       "loongarch64": 'Check these assembles to see if it need to port.'}
                      }
}
INLINE_ASSEMBLES = {'asm': {'description': '',
                            'suggestion': ''},
                    '__asm': {'description': '',
                              'suggestion': ''},
                    '__asm__': {'description': '',
                                'suggestion': ''},
                    '_asm': {'description': '',
                             'suggestion': ''}}
INLINE_ASSEMBLES_QUALIFIERS = {'volatile': {'description': '',
                                            'suggestion': ''},
                               '_volatile_': {'description': '',
                                              'suggestion': ''},
                               '__volatile__': {'description': '',
                                                'suggestion': ''},
                               'inline': {'description': '',
                                          'suggestion': ''},
                               'goto': {'description': '',
                                        'suggestion': ''}}
