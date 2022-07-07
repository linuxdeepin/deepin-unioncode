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

"""cli entry"""

import argparse
from importlib import import_module
import os
import sys
import threading
import time

from tool.tool_config import ToolConfig
from tool.util.logger import Logger

LOGGER = Logger().getlogger()

def config_tool():
    # python script directory.
    ToolConfig.tool_dir = os.path.dirname(os.path.abspath(__file__))
    # dict directory.
    ToolConfig.config_dir = os.path.join(ToolConfig.tool_dir, "config")
    user_home = os.path.expanduser('~')

    # report and source path, create it if not exists.
    ToolConfig.work_dir = os.path.join(user_home, 'UnionCode_Porting')
    ToolConfig.dirs.update(
        {"source_dir": os.path.join(ToolConfig.work_dir, 'sources'),
        "log_dir":os.path.join(ToolConfig.work_dir, 'logs'),
        "report_dir":os.path.join(ToolConfig.work_dir, 'reports')
    })
    for path in ToolConfig.dirs.values():
        if not os.path.exists(path):
            os.makedirs(path)

def args():
    """
    -S source path
    --cpu CPU Type
    """
    '''return command args'''
    parser = argparse.ArgumentParser()
    parser.add_argument('-S', '--source', dest='src', required=False, default=None, help='source foleder')
    parser.add_argument('--dcpu', dest='dcpu', required=True, help='architecture of dest cpu')
    parser.add_argument('--scpu', dest='scpu', required=False, default='x86_64', help='architecture of source cpu')
    args = parser.parse_args()
    if args.src is None:
        print("usage: [-h] [-S SRC] --cpu CPU \n error: the following arguments are requered: -S")
        exit(1)

    if  args.scpu.lower() not in ToolConfig.SUPPORT_CPU_ARCH or args.dcpu.lower() not in ToolConfig.SUPPORT_CPU_ARCH:
        LOGGER.error(
            "usage:cpu type [%s or %s] not supported, %s supported." % (args.scpu.lower(), args.dcpu.lower(), ToolConfig.SUPPORT_CPU_ARCH)
        )
        exit(1)

    return {"src": args.src, "src_cpu": args.scpu, "dest_cpu": args.dcpu}

def get_progress():
    i = 0
    while i <= 100:
        print("\r", end="")
        print("Scan progress [{}]: {}%: ".format("this is output", i),
        "▋" * (i // 2), end="")
        sys.stdout.flush()
        time.sleep(0.02)
        i += 1

def scaning(argumnets):
    t = threading.Thread(target=get_progress)
    t.start()

    # Wait until the thread terminates
    t.join()


if __name__ == '__main__':
    config_tool()
    Logger.config(ToolConfig.dirs["log_dir"], ToolConfig.log_name, 'WARN', 'DEBUG')
    cmdArguments = args()
    print(cmdArguments)
    LOGGER.info("info test")
    LOGGER.debug("debug test")
    scaning(cmdArguments)
    



