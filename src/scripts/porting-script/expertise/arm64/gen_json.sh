#
 # Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 #
 # Author:     luzhen<luzhen@uniontech.com>
 #
 # Maintainer: zhengyouge<zhengyouge@uniontech.com>
 #             luzhen<luzhen@uniontech.com>
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#!/bin/bash

instructions=(A64-general-instructions.txt \
A64-data-transfer-instructions.txt A64-Floating-point.txt \
A64-Advanced-SIMD-scalar.txt A64-Advanced-SIMD-vector.txt)

extract_instruction()
{
    for instruct in ${instructions[*]}
    do
        cat $instruct | awk -F '\t' '{print $1}' | awk -F '(' '{print $1}' | uniq  >> arm_inst.tmp
    done
}

echo "start extract_instruction."
extract_instruction
echo "finished."

python3 gen_json.py
rm -f arm_inst.tmp
