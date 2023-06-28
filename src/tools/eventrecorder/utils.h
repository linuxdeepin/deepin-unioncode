// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include "ScopedFd.h"

class StringVectorToCharArray {
public:
  StringVectorToCharArray(const std::vector<std::string>& vs) {
    for (auto& v : vs) {
      array.push_back(const_cast<char*>(v.c_str()));
    }
    array.push_back(nullptr);
  }
  char** get() { return array.data(); }

private:
  std::vector<char*> array;
};

ssize_t read_to_end(const ScopedFd& fd, size_t offset, void* buf, size_t size);
uint32_t crc32(uint32_t crc, unsigned char* buf, size_t len);
size_t page_size();
size_t ceil_page_size(size_t sz) ;

bool has_effective_caps();
bool running_under_rr();
const char* tmp_dir();

const char* ptrace_cmd_name(int request);

#endif /*end #ifndef _UTILS_H*/
