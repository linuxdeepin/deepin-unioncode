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
#include "symbolcppgenerator.h"
#include "symbolcppasynparser.h"

namespace  {

enum_def(MimeHdr, QString)
{
    enum_exp CXX_hdr {"text/x-c++hdr"};
    enum_exp C_hdr {"text/x-chdr"};
};

enum_def(MimeSrc, QString)
{
    enum_exp CXX = {"text/x-c++src"};
    enum_exp C = {"text/x-csrc"};
};

}

class SymbolCppGeneratorPrivate
{
    friend class SymbolCppGenerator;
    QThread *thread {nullptr};
    QStandardItem *currentRootItem {nullptr};
    QHash<QStandardItem*, QPair<QThreadPool*, QPair<int, int>>> itemAsynThreadPolls;
    QSet<SymbolCppAsynParser*> parsers;
};

QSet<QString> scanfSubDirs(const QString &filePath)
{
    QDir dir(filePath);
    dir.setSorting(QDir::SortFlag::Name);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    QDirIterator itera(dir, QDirIterator::Subdirectories);
    QSet<QString> subdirs {filePath};
    while (itera.hasNext()) {
        itera.next();
        subdirs << itera.filePath();
    }

    return subdirs;
}

QSet<QString> scanfSubFilesMap(const QString &dirPath) {
    QMimeDatabase mimedb;
    QDir dir(dirPath);
    dir.setSorting(QDir::SortFlag::Name);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
    QDirIterator itera(dir, QDirIterator::Subdirectories);
    QSet<QString> files {};
    while (itera.hasNext()) {
        itera.next();
        auto mimeType = mimedb.mimeTypeForFile(itera.fileInfo());
        if (::MimeHdr::contains(mimeType.name())
                || ::MimeSrc::contains(mimeType.name())) {
            files << itera.filePath();
        }
    }
    return files;
}

void scanfSubFilesReduce(QSet<QString> &result, const QSet<QString> &files)
{
    result += files;
}

SymbolCppGenerator::SymbolCppGenerator()
    : d (new SymbolCppGeneratorPrivate)
{

}

SymbolCppGenerator::~SymbolCppGenerator()
{
    qInfo() << __FUNCTION__;

    for (auto &val : d->itemAsynThreadPolls.keys()) {
        auto threadPoll = d->itemAsynThreadPolls[val].first;
        if (threadPoll) {
            threadPoll->clear();
            SymbolCppAsynParser::setGlobalRunFlags(false);
            while (threadPoll->activeThreadCount() != 0) {
                threadPoll->waitForDone(100);
            }
            delete thread();
        }
    }

    d->itemAsynThreadPolls.clear();

    if (d)
        delete d;
}

QStandardItem *SymbolCppGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return nullptr;

    QFileInfo fileInfo(info.workspaceFolder());
    auto root = new QStandardItem(fileInfo.fileName());

    // 关联item线程池

    dpfservice::ProjectInfo::set(root, info);
    auto files = info.sourceFiles();

    d->itemAsynThreadPolls[root].first = new QThreadPool();
    d->itemAsynThreadPolls[root].second.first = files.size();
    d->itemAsynThreadPolls[root].second.second = 0;

    for (auto file : files) {

        auto parser = new SymbolCppAsynParser();

        QObject::connect(parser, &SymbolCppAsynParser::parserEnd, [=](bool){
            delete parser;
            int allCount = d->itemAsynThreadPolls[root].second.first;
            int currentCount = d->itemAsynThreadPolls[root].second.second;
            d->itemAsynThreadPolls[root].second.second = currentCount ++;
            // remove threadPoll
            if (allCount == currentCount) {
                auto threadPoll = d->itemAsynThreadPolls[root].first;
                delete threadPoll;
                d->itemAsynThreadPolls.remove(root);
                emit Generator::finished(true);
            }
        });

        QtConcurrent::run(d->itemAsynThreadPolls[root].first,
                          parser, &SymbolCppAsynParser::doParserOne,
                          root, file, files);
    }

    emit Generator::started();

    return root;
}

void SymbolCppGenerator::removeRootItem(QStandardItem *root)
{
    auto threadPoll = d->itemAsynThreadPolls[root].first;
    if (threadPoll) {
        threadPoll->clear();
        SymbolCppAsynParser::setGlobalRunFlags(false);
        while(threadPoll->waitForDone(3000));
        delete threadPoll;
        d->itemAsynThreadPolls.remove(root);
        SymbolCppAsynParser::setGlobalRunFlags(true);
    }

    recursionRemoveItem(root);
}

QSet<QString> SymbolCppGenerator::scanfWorkspaceFiles(const QString &workspaceFolder)
{
    // one thread
    auto future = QtConcurrent::run(scanfSubDirs, workspaceFolder);
    future.waitForFinished(); // wait thread end
    // more thread
    return QtConcurrent::blockingMappedReduced(
                future.result(),
                scanfSubFilesMap,
                scanfSubFilesReduce);
}
