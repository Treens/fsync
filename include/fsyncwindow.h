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
#ifndef FSYNCWINDOW_H
#define FSYNCWINDOW_H

#include <QLineEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QWidget>

#include "ftree.h"

namespace Ui {
    class FsyncWindow;
}

class FsyncWindow : public QWidget
{
    Q_OBJECT

    public:
        explicit FsyncWindow(QWidget *parent = 0);
        ~FsyncWindow();

    public slots:
        void browseSourceFolder();
        void browseSaveFolder();

        void analyze();
        void endAnalyze();
        void save();
        void endSave();
        void cancelAnalyze();
        void cancelSave();

        void incrProgress();
        void setCurrentItem(const QString&);

        void disableUi();
        void enableUi();

        void updateTime();

    private:
        QTimer* timer;
        Ui::FsyncWindow *ui;
        Ftree* root;
        bool cancel;
        int time;

        void resetUi();
        void browseFolder(QLineEdit&, const char*);
        void addRow(QTableWidgetItem*, QTableWidgetItem*, QTableWidgetItem*);
        void createTable(Ftree*);
};

#endif // FSYNCWINDOW_H
