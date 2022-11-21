"""
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

import argparse
import os

from wordcount import WordCounter


def add_arguments(parser, proxy):
    parser.add_argument(
        "-w", "--workspace", type=str, default=proxy.workspace(),
        help="language source files root path, same LSP workspace, default is program path."
    )
    parser.add_argument(
        "-s", "--storage", type=str, required=True,
        help="storage path from do parse language files result saved"
    )
    parser.add_argument(
        "-l", "--language", default="", choices=proxy.supportLanguages(),
        help="select workspace language type files. It is blank by default. "
             "If no language is specified, all supported languages will be processed.",
    )
    return parser


def main():
    proxy = WordCounter()
    args_parser = add_arguments(argparse.ArgumentParser(), proxy)
    args = args_parser.parse_args()

    proxy.setWorkspace(args.workspace)
    proxy.setStorage(args.storage)
    if args.language:
        proxy.setLanguage(args.language)

    print(proxy.workspace(), proxy.storage(), proxy.language())
    proxy.doWordcount()

    # proxy.setWorkspace("/home/hjc/Downloads/deepin-draw-dev-v20/deepin-draw-dev-v20/.unioncode/symbol/Cxx")
    # proxy.setStorage("/home/hjc/Downloads/deepin-draw-dev-v20/deepin-draw-dev-v20/.unioncode/symbol/")
    # proxy.setLanguage('C/C++')
    # proxy.doWordcount()


if __name__ == '__main__':
    main()
