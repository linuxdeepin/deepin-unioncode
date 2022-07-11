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

# """write result [(src,linenum,keyword),(src1,linenum,keyword),....] into csv file."""
import csv
import os

from tool.tool_config import ToolConfig
from tool.util.logger import Logger

LOGGER = Logger().getlogger()


class CsvReport:
    def __init__(self):
        LOGGER.debug("csv report ")

    def generate(self, task_path, report_suffix, scan_results):
        report_file = 'porting-advisor-' + report_suffix + '.csv'
        try:
            if not os.path.exists(os.path.join(ToolConfig.dirs["report_dir"], task_path)):
                os.makedirs(os.path.join(ToolConfig.dirs["report_dir"], task_path))
            file_path = os.path.join(ToolConfig.dirs["report_dir"], task_path, report_file).replace('\\', '/')
            LOGGER.info("porting advisor for %s : %s" % (report_suffix, file_path))
            with open(file_path, 'w+', newline='', encoding='utf-8') as (file_desc):
                csv_write = csv.writer(file_desc, quoting=1)
                for result in scan_results:
                    csv_write.writerow(result)
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

    def readcsv(self, dir_path):
        file_path = os.path.join(dir_path, 'porting-advisor.csv').replace('\\', '/')
        try:
            with open(file_path, 'r') as (file_desc):
                csv_content = csv.DictReader(file_desc)
                for row in csv_content:
                    dict[row['src']] = row['src']
                    dict[row['linenum']] = row['linenum']
                    dict[row['keyword']] = row['keyword']

            return dict

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