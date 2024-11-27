# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import logging
import argparse
import os
from typing import Dict, List
from base_toolchain import ToolchainInfo
from toolchain_factory import ToolchainFactory

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class ToolchainScanner:
    def __init__(self, config_file: str):
        self.factory = ToolchainFactory(config_file)
        self.toolchains: Dict[str, List[ToolchainInfo]] = {
            t: [] for t in self.factory.get_supported_types()
        }
    
    def scan_system(self) -> None:
        for toolchain_type in self.toolchains.keys():
            try:
                scanner = self.factory.create_toolchain(toolchain_type)
                self.toolchains[toolchain_type] = scanner.scan()
                logger.info(f"Found {len(self.toolchains[toolchain_type])} {toolchain_type}")
            except Exception as e:
                logger.error(f"Error scanning {toolchain_type}: {str(e)}")
    
    def save_config(self, output_path: str) -> None:
        formatted_toolchains = {}
        for toolchain_type, tools in self.toolchains.items():
            formatted_toolchains[toolchain_type] = [
                {"name": tool.name, "path": tool.path} for tool in tools
            ]
        
        try:
            with open(output_path, 'w', encoding='utf-8') as f:
                json.dump(formatted_toolchains, f, indent=4, ensure_ascii=False)
            logger.info(f"Configuration saved to {output_path}")
        except Exception as e:
            logger.error(f"Error saving configuration: {str(e)}")

def parse_args():
    # Get the directory where main.py is located
    current_dir = os.path.dirname(os.path.abspath(__file__))
    default_config = os.path.join(current_dir, 'toolchain_config.json')
    default_output = os.path.join(current_dir, 'toolchains.json')

    parser = argparse.ArgumentParser(description='Scan system for development toolchains')
    parser.add_argument(
        '--output', '-o',
        type=str,
        default=default_output,
        help='Path to save the toolchains configuration (default: toolchains.json in current directory)'
    )
    parser.add_argument(
        '--config', '-c',
        type=str,
        default=default_config,
        help='Path to toolchain configuration file (default: toolchain_config.json in current directory)'
    )
    return parser.parse_args()

def main():
    args = parse_args()
    scanner = ToolchainScanner(args.config)
    scanner.scan_system()
    scanner.save_config(args.output)

if __name__ == "__main__":
    main()

