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
#ifndef FTREE_H
#define FTREE_H

#include <list>
#include <QDir>
#include <QFileInfo>

class Ftree;

class Ftree {
    public:
        Ftree(const QDir&, const QDir&);
        ~Ftree();

        int getChangeCount();

        void addChild(Ftree*);
        void setDirList(std::list<QFileInfo>*);
        void setFileList(std::list<QFileInfo>*);
        void setRemList(std::list<QFileInfo>*);

        const QDir* getMaster() const;
        const QDir* getSlave() const;
        const QList<Ftree*>* getChildren() const;
        const std::list<QFileInfo>* getDirList() const;
        const std::list<QFileInfo>* getFileList() const;
        const std::list<QFileInfo>* getRemList() const;

    private:
        QList<Ftree*> children;
        QDir master, slave;

        std::list<QFileInfo> *toAddDirs, *toAddFiles, *toRemove;
};

#endif // FTREE_H
