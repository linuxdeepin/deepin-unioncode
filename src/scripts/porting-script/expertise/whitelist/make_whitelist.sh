#!/bin/bash
# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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
