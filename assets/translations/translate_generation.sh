# Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
#
# Author:     hongjinchuan<hongjinchuan@uniontech.com>
#
# Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

cd ../../
#if [ ! -d "/translations/" ];then
#  mkdir /translations
lupdate  src/ -ts -no-obsolete assets/translations/*.ts

desk_ts_list=(`ls assets/translations/desktop/*.ts`)
for ts in "${desk_ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done

ts_list=(`ls assets/translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done