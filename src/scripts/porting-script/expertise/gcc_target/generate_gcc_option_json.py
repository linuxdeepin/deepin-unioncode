"""
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
"""
import json
import os

# TODO(mozart):get from toolconfig 
archSupport = ['arm64', 'x86_64', 'mips64el', 'sw_64']

def generate_x86_64(fo):
    mydict = {}

    for line in fo.readlines():
        line = line.strip()    
        subdata = {}
        suggestion_arch = {}
        suggestion_arch['arm64'] = common_suggestion('arm64', line)
        suggestion_arch['mips64el'] = common_suggestion('mips64el', line)
        suggestion_arch['sw_64'] = common_suggestion('sw_64', line)
        subdata['suggestion'] = suggestion_arch
        range = []
        subdata['range'] = range

        mydict[line] = subdata

    return mydict

def generate_arm64(fo):
    mydict = {}

    fo = open("optionlist", "r")
    for line in fo.readlines():
        line = line.strip()    
        subdata = {}
        suggestion_arch = {}
        suggestion_arch['x86_64'] = common_suggestion('x86_64', line)
        suggestion_arch['mips64el'] = common_suggestion('mips64el', line)
        suggestion_arch['sw_64'] = common_suggestion('sw_64', line)
        subdata['suggestion'] = suggestion_arch
        range = []
        subdata['range'] = range

        mydict[line] = subdata

    return mydict

def generate_mips64el(fo):
    mydict = {}

    for line in fo.readlines():
        line = line.strip()    
        subdata = {}
        suggestion_arch = {}
        suggestion_arch['arm64'] = common_suggestion('arm64', line)
        suggestion_arch['x86_64'] = common_suggestion('x86_64', line)
        suggestion_arch['sw_64'] = common_suggestion('sw_64', line)
        subdata['suggestion'] = suggestion_arch
        range = []
        subdata['range'] = range

        mydict[line] = subdata

    return mydict

def generate_sw_64(fo):
    mydict = {}

    for line in fo.readlines():
        line = line.strip()    
        subdata = {}
        suggestion_arch = {}
        suggestion_arch['arm64'] = common_suggestion('arm64', line)
        suggestion_arch['x86_64'] = common_suggestion('x86_64', line)
        suggestion_arch['mips64el'] = common_suggestion('mips64el', line)
        subdata['suggestion'] = suggestion_arch
        range = []
        subdata['range'] = range

        mydict[line] = subdata

    return mydict

def architecture():
    f = os.popen("arch") 
    arch = f.read().strip()
    return arch

def dictionary_path():
    return "GCC_8_3/" + architecture() + "_GCC_OPTION.json"

def save_to_file(dict):
    jsondata = json.dumps(dict)
    with open(dictionary_path(), "w") as fp:
        fp.write(jsondata)

def generate_dictionary(arch, fo):
    return globals()[f'generate_{arch}'](fo)

def common_suggestion(arch, option):
    '''just return common suggestion, more precise suggestions are not supported yet.'''
    return "Please check the %s option on %s platform" % (option, arch)

def main():
    arch = architecture()
    if arch in archSupport:
        fo = open("optionlist", "r")
        dict = generate_dictionary(arch, fo)
        fo.close()
        save_to_file(dict)
    else:
        print("only support %s architecture", archSupport)

if __name__ == '__main__':
    main()
