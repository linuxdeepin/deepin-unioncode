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

dicts = {}
instructions = {}
per_inst = {}
suggestion = {"suggestion": {"arm64": "todo", "mips64el": "todo", "sw_64": "todo"}}

def save_dict_to_json():
    json_data = json.dumps(instructions)
    with open("ARM64_ASM.json", "w") as (files_desc):
        files_desc.write(json_data)

def gen_arm_asm_inst_from_official():
    arm_insts = []
    with open("arm_inst.tmp") as file_desc:
        for line in file_desc:
            insts=line.strip('\n').replace(' ','').split(',')
            arm_insts.extend(insts)
    for inst in arm_insts:
          per_inst.update({inst: suggestion})
    instructions.update({"instructions": per_inst})

if __name__ == '__main__':
    gen_arm_asm_inst_from_official()
    save_dict_to_json()
