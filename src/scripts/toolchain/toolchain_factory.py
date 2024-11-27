# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import json
import os
import importlib.util
import inspect
from typing import Dict, Type, Callable, List, Tuple
from base_toolchain import BaseToolchain

class ToolchainFactory:
    def __init__(self, config_file: str = "toolchain_config.json"):
        self._creators: Dict[str, Callable[[], BaseToolchain]] = {}
        self._parsers = self._load_parsers()
        self._load_config(config_file)
    
    def _load_parsers(self) -> Dict[str, Type[BaseToolchain]]:
        """Dynamically load all parser classes from the parsers directory"""
        parsers = {}
        parsers_dir = os.path.join(os.path.dirname(__file__), 'parsers')
        
        # Iterate through all .py files in parsers directory
        for filename in os.listdir(parsers_dir):
            if filename.endswith('.py'):
                module_name = filename[:-3]  # Remove .py extension
                module_path = os.path.join(parsers_dir, filename)
                
                # Use importlib.util to dynamically import module
                spec = importlib.util.spec_from_file_location(module_name, module_path)
                if spec and spec.loader:
                    module = importlib.util.module_from_spec(spec)
                    spec.loader.exec_module(module)
                    
                    # Find all classes in the module
                    for name, obj in inspect.getmembers(module, inspect.isclass):
                        # Check if it's a subclass of BaseToolchain but not BaseToolchain itself
                        if (issubclass(obj, BaseToolchain) and 
                            obj != BaseToolchain and 
                            obj.__module__ == module_name):
                            parsers[name] = obj
        
        return parsers
    
    def _load_config(self, config_file: str):
        with open(config_file, 'r') as f:
            config = json.load(f)
        
        for toolchain in config['toolchains']:
            toolchain_type = toolchain['type']
            parser_class = self._parsers[toolchain['parser']]
            
            if 'tools' in toolchain:
                if parser_class.__name__ == 'SimpleToolchain':
                    # For SimpleToolchain, only pass the tool name
                    tool_name = next(iter(toolchain['tools'].keys()))
                    self._creators[toolchain_type] = lambda n=tool_name: self._parsers['SimpleToolchain'](n)
                else:
                    # For other parsers that need complete tool configuration
                    self._creators[toolchain_type] = lambda t=toolchain['tools'], p=parser_class: p(t)
            else:
                # No tools configuration, use parser class directly
                self._creators[toolchain_type] = parser_class
    
    def get_supported_types(self) -> List[str]:
        """Get all supported toolchain types"""
        return list(self._creators.keys())
    
    def create_toolchain(self, toolchain_type: str) -> BaseToolchain:
        creator = self._creators.get(toolchain_type)
        if not creator:
            raise ValueError(f"Unknown toolchain type: {toolchain_type}")
        return creator() 