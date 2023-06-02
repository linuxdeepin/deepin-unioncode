# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

class SyntaxTreeNode:
    src_file_path = ""
    prefix_len = 0
    type = ""
    string = ""
    location = []
    parent = None
    child = list()


def findParent(root: SyntaxTreeNode, prefix_len: int) -> SyntaxTreeNode:
    if prefix_len < 0:
        return None

    if prefix_len == 0:
        return root

    if prefix_len == root.prefix_len:
        return root.parent

    for node in reversed(root.child):
        return findParent(node, prefix_len)

    return root


def find_nodes_by_type(node: SyntaxTreeNode, type: str, recursion: bool = True):
    ret = []
    if not node:
        return ret

    if node.type == type:
        ret.append(node)

    for one in node.child:
        if recursion:
            ret.extend(find_nodes_by_type(one, type))
        else:
            if one.type == type:
                ret.append(one)
    return ret


def get_source_file_path(node: SyntaxTreeNode):
    if node.parent is None:
        return node.src_file_path
    else:
        return get_source_file_path(node.parent)
