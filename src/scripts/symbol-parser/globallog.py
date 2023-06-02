# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import logging
import time
import json

LOG_FORMAT = "%(asctime)s {0} - %(levelname)s - %(name)s - %(message)s".format(
    time.localtime().tm_zone)

def configure_logger(verbose=0, log_config=None, log_file=None):
    root_logger = logging.root

    if log_config:
        with open(log_config, 'r', encoding='utf-8') as f:
            logging.config.dictConfig(json.load(f))
    else:
        formatter = logging.Formatter(LOG_FORMAT)
        if log_file:
            log_handler = logging.handlers.RotatingFileHandler(
                log_file, mode='a', maxBytes=50*1024*1024,
                backupCount=10, encoding=None, delay=0
            )
        else:
            log_handler = logging.StreamHandler()
        log_handler.setFormatter(formatter)
        root_logger.addHandler(log_handler)

    if verbose == 0:
        level = logging.WARNING
    elif verbose == 1:
        level = logging.INFO
    elif verbose >= 2:
        level = logging.DEBUG

    root_logger.setLevel(level)

def log(moduleName = __name__):
    return logging.getLogger(moduleName)
