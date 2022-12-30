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