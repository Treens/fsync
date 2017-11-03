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
#include "applyworker.h"

ApplyWorker::ApplyWorker(Ftree* root) : root(root), cancel(false)
{}

void ApplyWorker::cancelWork() {
    cancel = true;
}

void ApplyWorker::run() {
    apply(root);
}

void ApplyWorker::apply(Ftree* tree) {
    for (auto it = tree->getRemList()->begin(); it != tree->getRemList()->end(); ++it) {
        if (cancel)
            return;
        if (it->isDir()) {
            emit itemChanged("Removing folder " + it->absoluteFilePath());
            QDir(it->absoluteFilePath()).removeRecursively();
        } else {
            //emit itemChanged("Suppression du fichier " + it->absoluteFilePath());
            QFile(it->absoluteFilePath()).remove();
        }

        emit progressed();
    }

    for (auto it = tree->getDirList()->begin(); it != tree->getDirList()->end(); ++it) {
        if (cancel)
            return;
        copyDir(QDir(it->absoluteFilePath()), tree->getSlave()->filePath(it->fileName()));

        emit progressed();
    }

    for (auto it = tree->getFileList()->begin(); it != tree->getFileList()->end(); ++it) {
        if (cancel)
            return;
        //emit itemChanged("Copie du fichier " + it->absoluteFilePath());
        QFile::copy(it->absoluteFilePath(), tree->getSlave()->filePath(it->fileName()));

        emit progressed();
    }

    for (auto it = tree->getChildren()->begin(); it != tree->getChildren()->end(); ++it)
        apply(*it);
}

void ApplyWorker::copyDir(const QDir& src, const QDir& dst) {
    emit itemChanged("Copying folder " + src.absolutePath());
    dst.mkpath(".");

    QFileInfoList fileList = src.entryInfoList(QDir::Dirs | QDir::Files |
                                               QDir::NoDotAndDotDot | QDir::NoSymLinks);

    for (auto it = fileList.begin(); it != fileList.end(); ++it) {
        if (cancel)
            return;
        if (it->isDir()) {
            copyDir(QDir(it->absoluteFilePath()), dst.filePath(it->fileName()));
        } else {
            //emit itemChanged("Copie du fichier " + it->absoluteFilePath());
            QFile::copy(it->absoluteFilePath(), dst.filePath(it->fileName()));
        }
    }
}
