# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""cli entry"""

import argparse
import os
import sys
import threading
import time
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
    ToolConfig.work_dir = os.path.join(ToolConfig.build_dir, 'UnionCode_Porting')
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
    count = 0
    run_in_terminal = sys.stdin and sys.stdin.isatty()
    while int(scan_api.progress.progress) <= 100:
        print('\r', end='')

        if run_in_terminal:
            print('Scan progress [{}]: {}%:'.format(scan_api.progress.info, int(scan_api.progress.progress)),
            "▋" * (int(scan_api.progress.progress) // 2), end="", flush=True)
        else:
            remainder = count % 4
            count += 1
            shapes = {0:'', 1:'.', 2:'..', 3:'...'}
            progressInfo = "Running task: {} {}".format(scan_api.progress.info, shapes.get(remainder))
            LOGGER.info(progressInfo)

        time.sleep(0.2)
        if int(scan_api.progress.progress) == 100:
            break

def start_scan_src(input_dict):
    scan_api = ScanApi(input_dict)
    t = threading.Thread(target=get_progress, args=(scan_api,))
    t.start()
    scan_api.import_rules()
    scan_api.start_scan()
    scan_api.progress.progress = 100
    t.join()


if __name__ == '__main__':
    cmdArguments = args()
    config_tool(cmdArguments)
    # Logger.config(ToolConfig.dirs["log_dir"], ToolConfig.log_name, 'WARN', 'DEBUG')
    Logger.config(ToolConfig.dirs["log_dir"], ToolConfig.log_name, 'INFO', 'INFO')
    start_scan_src(cmdArguments)
    



