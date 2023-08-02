# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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
