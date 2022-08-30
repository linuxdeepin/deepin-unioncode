"""
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
"""

ASSEMBLES_DICT = {
    '__assembles__': {'description': "Found arch specific assembles using '__asm'/'__asm__'/'asm'/'_asm'.",
                      'suggestion':
                      {"arm64": 'Check these assembles to see if it need to port.',
                       "mips64el": 'Check these assembles to see if it need to port.',
                       "x86_64": 'Check these assembles to see if it need to port.',
                       "sw_64": 'Check these assembles to see if it need to port.'}
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
