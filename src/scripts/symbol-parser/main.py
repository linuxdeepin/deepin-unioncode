# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#!/usr/bin/env python3.7
# coding: utf-8

import argparse
import os
import sys

from multiprocessing import cpu_count

from globallog import configure_logger
from globallog import log
from callproxy import CallProxy
from cxxparser import CxxParser
from javaparser import JavaParser
from pythonparser import PythonParser
from javascriptparser import JavascriptParser


def add_arguments(parser, proxy):
    parser.add_argument(
        "-s", "--storage", type=str, required=True,
        help="storage path from do parse language files result saved"
    )
    parser.add_argument(
        "-w", "--workspace", type=str, default=proxy.workspace(),
        help="language source files root path, same LSP workspace, default is program path."
    )
    parser.add_argument(
        "-l", "--language", default="", choices=proxy.supportLanguages(),
        help="select workspace language type files. It is blank by default. "
             "If no language is specified, all supported languages will be processed.",
    )
    parser.add_argument(
        "-j", "--job", type=int, default=cpu_count(),
        help="The number of tasks to be started is generally not required to be specified. "
             "By default, it is consistent with the number of CPUs"
    )
    parser.add_argument(
        '-v', '--verbose', action='count', default=0,
        help="Increase verbosity of log output, overrides log config file."
    )
    log_group = parser.add_mutually_exclusive_group()
    log_group.add_argument(
        "--log-config",
        help="Path to a JSON file containing Python logging config."
    )
    log_group.add_argument(
        "--log-file",
        help="Redirect logs to the given file instead of writing to stderr."
             "Has no effect if used with --log-config."
    )
    return parser


def _stdio():
    stdin, stdout = sys.stdin.buffer, sys.stdout.buffer
    return stdin, stdout


def initProxy():
    proxy = CallProxy()
    proxy.addParser(CxxParser())
    proxy.addParser(JavaParser())
    proxy.addParser(PythonParser())
    proxy.addParser(JavascriptParser())
    return proxy


def getFiles(workspace):
    files = []
    for root, dirs, files in os.walk(workspace):
        root + "\\" + files
        print(root)
        print(dirs)
        print(files)


def main():
    proxy = initProxy()
    args_parser = add_arguments(argparse.ArgumentParser(), proxy)
    args_parser.description = "Union Parser"
    args = args_parser.parse_args()
    configure_logger(args.verbose, args.log_config, args.log_file)
    log(__name__).info("started " + args_parser.description)
    proxy.setWorkspace(args.workspace)
    proxy.setStorage(args.storage)
    if args.language:
        proxy.setLanguage(args.language)
    proxy.doParse()


if __name__ == '__main__':
    main()
