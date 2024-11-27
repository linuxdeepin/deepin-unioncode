# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from abc import ABC, abstractmethod
from typing import List, Dict
import os
import glob
from dataclasses import dataclass

@dataclass
class ToolchainInfo:
    name: str
    path: str

class BaseToolchain(ABC):
    def __init__(self):
        self._executables: List[str] = []
    
    def find_tools(self, tool_name: str) -> List[str]:
        tools = []
        paths = os.environ.get("PATH", "").split(os.pathsep)
        
        priority_paths = ['/usr/bin', '/bin']
        other_paths = [p for p in paths if p not in priority_paths]
        scan_paths = priority_paths + other_paths
        
        for path in scan_paths:
            if not os.path.exists(path):
                continue
            matches = glob.glob(os.path.join(path, tool_name))
            for match in matches:
                if os.access(match, os.X_OK | os.F_OK):
                    tools.append(match)
        return self.auto_detect_toolchains(tools)
    
    def auto_detect_toolchains(self, compiler_paths: List[str]) -> List[str]:
        result = []
        for compiler_path in compiler_paths:
            already_exists = False
            for existing_tc in result:
                already_exists = os.lstat(existing_tc).st_ino == os.lstat(compiler_path).st_ino
                if already_exists:
                    break
            if not already_exists:
                result.append(compiler_path)
        return result

    @abstractmethod
    def scan(self) -> List[ToolchainInfo]:
        pass 