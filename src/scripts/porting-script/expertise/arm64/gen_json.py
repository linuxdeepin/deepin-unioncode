# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import json

dicts = {}
instructions = {}
per_inst = {}
suggestion = {"suggestion": {"arm64": "todo", "mips64el": "todo", "sw_64": "todo", "loongarch64": "todo"}}

def save_dict_to_json():
    json_data = json.dumps(instructions)
    with open("../../config/ASM/ARM64_ASM.json", "w") as (files_desc):
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
