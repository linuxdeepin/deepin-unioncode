#!/bin/bash
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

arch=`arch`

declare -A optionNums
set optionNum=0

optionNums=(["x86_64"]=199 ["aarch64"]=24 ["mips64"]=106 ["sw_64"]=62 ["loongarch64"]=53)
optionNum=${optionNums[$arch]}

echo $optionNum

gcc --target-help | sed $optionNum,'$d' | sed '1d'  | sed 's/^$//g' | sed 's/^  //g' | cut -d ' ' -f 1 > optionlist
echo "start generate dictionary"
python3 generate_gcc_option_json.py
echo "done."