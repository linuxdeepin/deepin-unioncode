#!/bin/bash
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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
