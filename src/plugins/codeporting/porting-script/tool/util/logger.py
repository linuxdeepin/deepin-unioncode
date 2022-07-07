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
"""logger"""

import logging
import logging.handlers
import os
import time
from tool.tool_config import ToolConfig

class Logger:
    # debug levels.
    LEVELS = {'NOTSET':logging.NOTSET,
    'DEBUG':logging.DEBUG,
    'INFO':logging.INFO,
    'WARNING':logging.WARNING,
    'ERROR':logging.ERROR}

    # logger = logging.getLogger('')

    @classmethod
    def config(cls, filepath, logger_name='', logfile_level='DEBUG', console_level='INFO'):
        cls.logs_dir = filepath
        cls.logger_name = logger_name
        cls.logs_file_level = logfile_level
        cls.logs_console_level = console_level

        # create logger dirctory.
        cls.logger = logging.getLogger(cls.logger_name)
        if os.path.exists(cls.logs_dir) and os.path.isdir(cls.logs_dir):
            pass
        else:
            os.mkdir(cls.logs_dir)

        # name log file.
        timestamp = time.strftime("%Y-%m-%d",time.localtime())
        logfilename = '%s.txt' % timestamp
        logfilepath = os.path.join(cls.logs_dir, logfilename)
        rotatingFileHandler = logging.handlers.RotatingFileHandler(filename=logfilename,
        maxBytes=1024*1024*50, backupCount=2)
        
        # set log formate
        formatter = logging.Formatter('[%(asctime)s] [%(filename)s] [%(lineno)s] [%(levelname)s] %(message)s', '%Y-%m-%d %H:%M:%S')
        rotatingFileHandler.setFormatter(formatter)

        consoleHandler = logging.StreamHandler()
        consoleHandler.setLevel(cls.logs_console_level)
        consoleHandler.setFormatter(formatter)

        # put handler to logger.
        cls.logger.addHandler(rotatingFileHandler)
        cls.logger.addHandler(consoleHandler)
        cls.logger.setLevel(cls.logs_file_level)

    def getlogger(cls):
        return logging.getLogger(ToolConfig.log_name)

# do test
if __name__ == '__main__':
    Logger.config('.', 'test')
    LOGGER = Logger().getlogger()
    LOGGER.info("info")
    LOGGER.debug("debug")
    LOGGER.error("error")
    LOGGER.warning("warning")




