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
#ifndef APPLYWORKER_H
#define APPLYWORKER_H

#include <QDir>
#include <QString>
#include <QThread>
#include "ftree.h"

class ApplyWorker : public QThread
{
    Q_OBJECT

    public:
        ApplyWorker(Ftree*);

    public slots:
        void cancelWork();

    signals:
        void itemChanged(QString);
        void progressed();

    private:
        Ftree* root;
        bool cancel;

        void run();
        void apply(Ftree*);
        void copyDir(const QDir&, const QDir&);
};

#endif
