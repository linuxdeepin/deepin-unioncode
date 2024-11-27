# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from typing import List
import os
import subprocess
from base_toolchain import BaseToolchain, ToolchainInfo

class JavaToolchain(BaseToolchain):
    def __init__(self):
        super().__init__()
        self.tool_name = "java"
    
    def get_java_version(self, java_path: str) -> str:
        try:
            process = subprocess.Popen(
                [java_path, "-version"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            _, stderr = process.communicate()
            
            first_line = stderr.split('\n')[0]
            parts = first_line.split()
            if len(parts) >= 3:
                version = parts[2].replace('"', '')
                return f"{parts[0]} {version}"
            return os.path.basename(java_path)
        except Exception as e:
            return os.path.basename(java_path)
    
    def scan(self) -> List[ToolchainInfo]:
        paths = self.find_tools(self.tool_name)
        return [
            ToolchainInfo(
                name=self.get_java_version(path),
                path=path
            ) for path in paths
        ] 