# SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from typing import List, Dict
import os
import glob
from base_toolchain import BaseToolchain, ToolchainInfo

class GccToolchain(BaseToolchain):
    def __init__(self, patterns: Dict[str, str]):
        super().__init__()
        self.patterns = patterns
    
    def find_compiler_candidates(self, executables: List[str], compiler_name: str) -> List[str]:
        compiler_paths = []
        cl = len(compiler_name)
        
        for executable in executables:
            file_name = os.path.basename(executable)
            if file_name == compiler_name:
                compiler_paths.append(executable)
                continue
            if file_name in ["c89-gcc", "c99-gcc"]:
                continue
            pos = file_name.find(compiler_name)
            if pos == -1:
                continue
            if pos > 0 and file_name[pos - 1] != '-':
                continue
                
            pos += cl
            if pos != len(file_name):
                if pos + 1 >= len(file_name):
                    continue
                c = file_name[pos + 1]
                if not c.isdigit():
                    continue
            compiler_paths.append(executable)
            
        return compiler_paths
    
    def scan(self) -> List[ToolchainInfo]:
        executables = []
        paths = os.environ.get('PATH', '').split(os.pathsep)
        priority_paths = ['/usr/bin', '/bin']
        other_paths = [p for p in paths if p not in priority_paths]
        scan_paths = priority_paths + other_paths
        
        for path in scan_paths:
            if not os.path.exists(path):
                continue
            for pattern in self.patterns.values():
                matches = glob.glob(os.path.join(path, pattern))
                for match in matches:
                    if os.access(match, os.X_OK | os.F_OK):
                        executables.append(match)
        
        toolchains = []
        for compiler_type in self.patterns.keys():
            candidates = self.find_compiler_candidates(executables, compiler_type)
            detected = self.auto_detect_toolchains(candidates)
            toolchains.extend(detected)
        
        return [
            ToolchainInfo(
                name=os.path.basename(path),
                path=path
            ) for path in toolchains
        ] 