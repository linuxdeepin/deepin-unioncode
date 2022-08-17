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

path='/var/lib/apt/lists'

gen_arm_whitelist()
{
    rm arm_whitelist
    ls $path/*arm64_Packages | xargs cat | grep '^Version:' | sed 's/^Version://g' | sed 's/^ //g' >> versions
    ls $path/*arm64_Packages | xargs cat | grep '^Package:' | sed 's/^Package://g' | sed 's/^ //g'>> packages
    paste -d ' ' packages versions > info
    rm versions packages
}

gen_mips64el_whitelist()
{
    rm mips_whitelist
    ls $path/*mips64el_Packages | xargs cat | grep '^Version:' | sed 's/^Version://g' | sed 's/^ //g' >> versions
    ls $path/*mips64el_Packages | xargs cat | grep '^Package:' | sed 's/^Package://g' | sed 's/^ //g'>> packages
    paste -d ' ' packages versions > info
    rm versions packages
}

gen_sw_64_whitelist()
{
    rm sw_64_whitelist
    ls $path/*sw%5f64_Packages | xargs cat | grep '^Version:' | sed 's/^Version://g' | sed 's/^ //g' >> versions
    ls $path/*sw%5f64_Packages | xargs cat | grep '^Package:' | sed 's/^Package://g' | sed 's/^ //g'>> packages
    paste -d ' ' packages versions > info
    rm versions packages
}

usage_tip()
{
    echo "usage：./make_whitelist.sh -d [arm64/mips64el/sw_64]"
}

if [[ -n $1 && -n $2 ]];then
    if [ $1 != "-d" ];then
        usage_tip
        exit -1
    fi
    if [ "arm64" == $2 ];then
        echo "Generating arm64 whitelist list..." 
        gen_arm_whitelist
    elif [ "mips64el" == $2 ];then
        echo "Generating mips64el whitelist list..." 
        gen_mips64el_whitelist
    elif [ "sw_64" == $2 ];then
        echo "Generating sw_64 whitelist list..."
        gen_mips64el_whitelist
    else
        usage_tip        
        exit -1
    fi
else
    usage_tip
fi
