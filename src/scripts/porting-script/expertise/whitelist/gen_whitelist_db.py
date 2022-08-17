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
#!/usr/bin/python

import sqlite3

conn = sqlite3.connect('code-porting.db')
print ('Opened database successfully')
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS WHITELIST
            (PACKAGE TEXT PRIMARY KEY  NOT NULL,
            VERSION TEXT);''')

c.execute('''CREATE TABLE IF NOT EXISTS PORTING_RESULT
            (TASKID TEXT PRIMARY KEY  NOT NULL,
            RESULT TEXT);''')

c.execute('''CREATE TABLE IF NOT EXISTS TASK_INFO
            (TASKID TEXT PRIMARY KEY  NOT NULL,
            TASKNAME TEXT);''')
c.execute('''CREATE TABLE IF NOT EXISTS BUILD_DEPENDS_TREE
            (TASKID TEXT PRIMARY KEY  NOT NULL,
            DEPENDS TEXT);''')

print ('Table created successfully')

# cursor = c.execute("SELECT COUNT(*) from WHITELIST")
# for len in cursor:
#     print(len)

file = open('info', 'r+')
while True:
    str = file.readline()
    if len(str) == 0:
        print('read over')
        break
    info = str.split(' ', 1)
    print(info[0])
    print(info[1])
    cursor = c.execute("SELECT VERSION from WHITELIST where PACKAGE='%s'" % info[0])
    flag = False
    for row in cursor:
        if len(row[0]) > 0:
            flag = True
        if len(row[0]) > 0 and info[1] > row[0]:
            c.execute("UPDATE WHITELIST set VERSION='%s' where PACKAGE='%s'" % (info[1], info[0]))
    if flag == False:
        c.execute("INSERT INTO WHITELIST (PACKAGE, VERSION) VALUES ('%s', '%s')" % (info[0], info[1]))

conn.commit()
conn.close()
