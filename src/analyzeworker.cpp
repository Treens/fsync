/*Copyright 2017 Pierre Franco
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*/
#include <queue>
#include <vector>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include "analyzeworker.h"

#define BUFFER_SIZE 4096
#define BLOCK_CHECK 128

AnalyzeWorker::AnalyzeWorker(Ftree* root) : root(root), cancel(false)
{}

void AnalyzeWorker::cancelWork() {
    cancel = true;
}

void AnalyzeWorker::run() {
    compare(root);
}

void AnalyzeWorker::compare(Ftree* tree) {
    std::list<QFileInfo>* masterFileList = new std::list<QFileInfo>(
            tree->getMaster()->entryInfoList(QDir::Files |
                                 QDir::NoDotAndDotDot | QDir::NoSymLinks).toStdList());
    std::list<QFileInfo>* masterDirList = new std::list<QFileInfo>(
            tree->getMaster()->entryInfoList(QDir::Dirs |
                                 QDir::NoDotAndDotDot | QDir::NoSymLinks).toStdList());
    std::list<QFileInfo>* slaveList = new std::list<QFileInfo>(
            tree->getSlave()->entryInfoList(QDir::Dirs | QDir::Files |
                                QDir::NoDotAndDotDot | QDir::NoSymLinks).toStdList());

    std::list<std::list<QFileInfo>::iterator> masterFileToRemove, masterDirToRemove;

    for (auto mit = masterFileList->begin(); mit != masterFileList->end(); ++mit) {
        emit itemChanged("Analysing file " + mit->absoluteFilePath());

        for (auto sit = slaveList->begin(); sit != slaveList->end(); ++sit) {
            if (cancel)
                return;
            if (sit->isFile() && compareFiles(*mit, *sit)) {
                masterFileToRemove.push_back(mit);
                slaveList->erase(sit);
                break;
            }
        }

    }

    for (auto mit = masterDirList->begin(); mit != masterDirList->end(); ++mit) {
        emit itemChanged("Analysing folder " + mit->absoluteFilePath());

        for (auto sit = slaveList->begin(); sit != slaveList->end(); ++sit) {
            if (cancel)
                return;
            if (sit->isDir() && mit->fileName() == sit->fileName()) {
                Ftree* child = new Ftree(QDir(mit->absoluteFilePath()),
                                         QDir(sit->absoluteFilePath()));
                compare(child);
                tree->addChild(child);
                masterDirToRemove.push_back(mit);
                slaveList->erase(sit);
                break;
            }
        }
    }

    for (auto it = masterFileToRemove.begin(); it != masterFileToRemove.end(); ++it) {
        if (cancel)
            return;
        masterFileList->erase(*it);
    }
    for (auto it = masterDirToRemove.begin(); it != masterDirToRemove.end(); ++it) {
        if (cancel)
            return;
        masterDirList->erase(*it);
    }

    tree->setFileList(masterFileList);
    tree->setDirList(masterDirList);
    tree->setRemList(slaveList);
}

bool AnalyzeWorker::compareFiles(const QFileInfo& f1, const QFileInfo& f2) {
    if (f1.fileName() != f2.fileName())
        return false;

    if (f1.size() != f2.size())
        return false;

    QFile f1Handle(f1.absoluteFilePath());
    QFile f2Handle(f2.absoluteFilePath());
    QByteArray buffer1, buffer2;
    const int blockCount = (f1.size() + (BUFFER_SIZE - 1))/BUFFER_SIZE;
    bool eq = true;

    if (!f1Handle.open(QIODevice::ReadOnly))
        return false;

    if (!f2Handle.open(QIODevice::ReadOnly)) {
        f1Handle.close();
        return false;
    }

    // Check first block
    buffer1 = f1Handle.read(BUFFER_SIZE);
    buffer2 = f2Handle.read(BUFFER_SIZE);

    if (buffer1 != buffer2)
        eq = false;

    //Check some random blocks
    if (eq && blockCount > 2) {
        std::priority_queue<qint64, std::vector<qint64>, std::greater<qint64>> blockList;

        qsrand(f1.size());

        int checkCount = (blockCount + BLOCK_CHECK - 1)/BLOCK_CHECK;

        for (qint64 i = 0; i < checkCount; ++i)
            blockList.push(((qrand() % (blockCount - 2)) + 2)*BUFFER_SIZE);

        for (qint64 block; !blockList.empty();) {
            block = blockList.top();
            blockList.pop();

            f1Handle.seek(block);
            f2Handle.seek(block);

            buffer1 = f1Handle.read(BUFFER_SIZE);
            buffer2 = f2Handle.read(BUFFER_SIZE);

            if (buffer1 != buffer2) {
                eq = false;
                break;
            }
        }
    }

    // Check last block
    if (eq) {
        f1Handle.seek((blockCount - 1)*BUFFER_SIZE);
        f2Handle.seek((blockCount - 1)*BUFFER_SIZE);

        buffer1 = f1Handle.read(BUFFER_SIZE);
        buffer2 = f2Handle.read(BUFFER_SIZE);

        if (buffer1 != buffer2)
            eq = false;
    }

    f1Handle.close();
    f2Handle.close();

    return eq;
}
