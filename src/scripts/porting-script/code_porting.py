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
import os
import sys
import threading
import time
from tool import tool_config
from tool.api.scan import ScanApi
from tool.tool_config import ToolConfig
from tool.util.logger import Logger

LOGGER = Logger().getlogger()

def config_tool(input_dict):
    ToolConfig.build_dir = input_dict['build_dir']
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
    -B build path
    --cpu CPU Type
    """
    '''return command args'''
    parser = argparse.ArgumentParser()
    parser.add_argument('-S', '--source', dest='src', required=True, default=None, help='source foleder')
    parser.add_argument('-B', '--build', dest='build', required=True, default=None, help='build output foleder')
    parser.add_argument('--dcpu', dest='dcpu', required=True, help='architecture of dest cpu')
    parser.add_argument('--scpu', dest='scpu', required=False, default='x86_64', help='architecture of source cpu')
    args = parser.parse_args()

    if  args.scpu.lower() not in ToolConfig.SUPPORT_CPU_ARCH or args.dcpu.lower() not in ToolConfig.SUPPORT_CPU_ARCH:
        LOGGER.error(
            "usage:cpu type [%s or %s] not supported, %s supported." % (args.scpu.lower(), args.dcpu.lower(), ToolConfig.SUPPORT_CPU_ARCH)
        )
        exit(1)

    return {"src": args.src, "build_dir": args.build, "src_cpu": args.scpu, "dest_cpu": args.dcpu}

def get_progress(scan_api):
    while int(scan_api.progress.progress) <= 100:
        print('\r', end='')
        print('Scan progress [{}]: {}%:'.format(scan_api.progress.info, int(scan_api.progress.progress)),
        "▋" * (int(scan_api.progress.progress) // 2), end="")
        sys.stdout.flush()
        time.sleep(0.05)

        progressInfo = "Scan progress %s %s %d%%" % (scan_api.progress.info, "▋" * (int(scan_api.progress.progress) // 2), int(scan_api.progress.progress))
        LOGGER.info(progressInfo)
        if int(scan_api.progress.progress) == 100:
            break

def start_scan_src(input_dict):
    scan_api = ScanApi(input_dict)
    t = threading.Thread(target=get_progress, args=(scan_api,))
    t.start()
    scan_api.import_rules()
    no_make_file = scan_api.start_scan()
    if len(no_make_file):
        LOGGER.warning('no makefile in [%s]' % no_make_file)
    # scan_api.scan_src()
    scan_api.progress.progress = 100
    t.join()


if __name__ == '__main__':
    cmdArguments = args()
    config_tool(cmdArguments)
    # Logger.config(ToolConfig.dirs["log_dir"], ToolConfig.log_name, 'WARN', 'DEBUG')
    Logger.config(ToolConfig.dirs["log_dir"], ToolConfig.log_name, 'INFO', 'INFO')
    print(cmdArguments)
    LOGGER.info("info test")
    start_scan_src(cmdArguments)
    



