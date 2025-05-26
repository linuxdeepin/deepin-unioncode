# SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from typing import List
import os
import glob
from base_toolchain import BaseToolchain, ToolchainInfo

class PythonToolchain(BaseToolchain):
    def __init__(self):
        super().__init__()
        self.filter_patterns = [
            "python",
            "python[1-9]",
            "python[1-9].[0-9]",
            "python[1-9].[1-9][0-9]"
        ]
    
    def scan(self) -> List[ToolchainInfo]:
        interpreters = []
        paths = os.environ.get("PATH", "").split(os.pathsep)
        
        for path in paths:
            if not os.path.exists(path):
                continue
            for pattern in self.filter_patterns:
                matches = glob.glob(os.path.join(path, pattern))
                for match in matches:
                    if os.access(match, os.X_OK | os.F_OK):
                        interpreters.append(os.path.realpath(match))
        
        detected_paths = self.auto_detect_toolchains(interpreters)
        return [
            ToolchainInfo(
                name=os.path.basename(path),
                path=path
            ) for path in detected_paths
        ] 