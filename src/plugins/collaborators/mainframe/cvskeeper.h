/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
*/
#ifndef CVSKEEPER_H
#define CVSKEEPER_H

#include <QObject>

class SvnClientWidget;
class GitQlientWidget;
class CVSkeeper final: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CVSkeeper)
    explicit CVSkeeper(QObject *parent = nullptr);

public:
    static CVSkeeper *instance();
    void openRepos(const QString &repoPath);
    SvnClientWidget *svnMainWidget();
    GitQlientWidget *gitMainWidget();

private:
    SvnClientWidget *svnReposWidget;
    GitQlientWidget *gitReposWidget;
};

#endif // CVSKEEPER_H
