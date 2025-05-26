# SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#!/usr/bin/env python3
import json
import subprocess
import sys

def get_gcc_include_paths():
    # Get the default include paths for g++
    cmd = "echo | g++ -xc++ -E -v - 2>&1"
    output = subprocess.check_output(cmd, shell=True).decode()
    
    # Parse output to get include paths
    paths = []
    collecting = False
    for line in output.split('\n'):
        if line == "#include <...> search starts here:":
            collecting = True
            continue
        elif line == "End of search list.":
            collecting = False
            continue
        
        if collecting:
            path = line.strip()
            if path:
                paths.append(f"-I{path}")
    
    return paths

def update_compile_commands(filepath):
    try:
        # Read the original file
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        # Get g++ include paths
        include_paths = get_gcc_include_paths()
        
        # Update each compile command
        for item in data:
            if 'command' in item:
                # Add include paths to existing command
                cmd_parts = item['command'].split()
                # Insert include paths before the first -I or source file
                insert_pos = 1  # Default to insert after the compiler name
                for i, part in enumerate(cmd_parts):
                    if part.startswith('-I') or part.endswith('.cpp') or part.endswith('.cc'):
                        insert_pos = i
                        break
                
                # Insert new include paths
                cmd_parts[insert_pos:insert_pos] = include_paths
                item['command'] = ' '.join(cmd_parts)
            
            elif 'arguments' in item:
                # If using arguments array format
                args = item['arguments']
                insert_pos = 1  # Default to insert after the compiler name
                for i, arg in enumerate(args):
                    if arg.startswith('-I') or arg.endswith('.cpp') or arg.endswith('.cc'):
                        insert_pos = i
                        break
                
                # Insert new include paths
                args[insert_pos:insert_pos] = include_paths
                item['arguments'] = args
        
        # Write back to file
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)
            
        print("Successfully updated compile_commands.json")
        
    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 update_compile_commands.py path/to/compile_commands.json")
        sys.exit(1)
    
    update_compile_commands(sys.argv[1])

