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
#include "ftree.h"

Ftree::Ftree(const QDir& master, const QDir& slave) :
    master(master), slave(slave),
    toAddDirs(nullptr), toAddFiles(nullptr), toRemove(nullptr)
{}

Ftree::~Ftree() {
    if (toAddDirs)
        delete toAddDirs;
    if (toAddFiles)
        delete toAddFiles;
    if (toRemove)
        delete toRemove;

    for (auto it = children.begin(); it != children.end(); ++it)
        delete *it;
}

int Ftree::getChangeCount() {
    int count = 0;

    if (toAddDirs)
        count += toAddDirs->size();
    if (toAddFiles)
        count += toAddFiles->size();
    if (toRemove)
        count += toRemove->size();

    for (auto it = children.begin(); it != children.end(); ++it)
        count += (*it)->getChangeCount();

    return count;
}

void Ftree::addChild(Ftree* child) {
    children.push_back(child);
}

void Ftree::setDirList(std::list<QFileInfo>* list) {
    toAddDirs = list;
}

void Ftree::setFileList(std::list<QFileInfo>* list) {
    toAddFiles = list;
}

void Ftree::setRemList(std::list<QFileInfo>* list) {
    toRemove = list;
}

const QDir* Ftree::getMaster() const {
    return &master;
}

const QDir* Ftree::getSlave() const {
    return &slave;
}

const QList<Ftree*>* Ftree::getChildren() const {
    return &children;
}


const std::list<QFileInfo>* Ftree::getDirList() const {
    return toAddDirs;
}

const std::list<QFileInfo>* Ftree::getFileList() const {
    return toAddFiles;
}

const std::list<QFileInfo>* Ftree::getRemList() const {
    return toRemove;
}
