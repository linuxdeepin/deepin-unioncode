# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from typing import List
import os
from base_toolchain import BaseToolchain, ToolchainInfo

class SimpleToolchain(BaseToolchain):
    def __init__(self, tool_name: str):
        super().__init__()
        self.tool_name = tool_name
    
    def scan(self) -> List[ToolchainInfo]:
        paths = self.find_tools(self.tool_name)
        return [
            ToolchainInfo(
                name=os.path.basename(path),
                path=path
            ) for path in paths
        ] 