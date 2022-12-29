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
set optionNum=0

if [ "$arch" = "x86_64" ];then
    optionNum=168
elif [ "$arch" = "aarch64" ];then
    optionNum=24
elif [ "$arch" = "mips64" ];then
    optionNum=106
elif [ "$arch" = "sw_64" ];then
    optionNum=100
fi

echo $optionNum

gcc --target-help | sed $optionNum,'$d' | sed '1d'  | sed 's/^$//g' | sed 's/^  //g' | cut -d ' ' -f 1 > optionlist
echo "start generate dictionary"
python3 generate_gcc_option_json.py
echo "done."