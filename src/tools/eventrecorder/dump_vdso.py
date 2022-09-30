#
 # Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 #
 # Author:     liuchanghui<liuchanghui@uniontech.com>
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

#!/usr/bin/env python
from ctypes import *

for ln in open('/proc/self/maps'):
    if "[vdso]" in ln:
        start, end = [int(x,16) for x in ln.split()[0].split('-')]
        CDLL("libc.so.6").write(1, c_void_p(start), end-start)
        break
