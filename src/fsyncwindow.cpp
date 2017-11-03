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
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>

#include "fsyncwindow.h"
#include "ui_fsyncwindow.h"
#include "analyzeworker.h"
#include "applyworker.h"

FsyncWindow::FsyncWindow(QWidget *parent) :
    QWidget(parent), timer(nullptr), ui(new Ui::FsyncWindow), root(nullptr), time(0)
{
    timer = new QTimer(this);

    ui->setupUi(this);

    ui->diffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->aboutLabel->setText("Fsync version " + QString(FSYNCVERSION) + " from " + QString(__DATE__));

    QObject::connect(ui->sourceBrowse, SIGNAL(pressed()), SLOT(browseSourceFolder()));
    QObject::connect(ui->saveBrowse, SIGNAL(pressed()), SLOT(browseSaveFolder()));

    QObject::connect(ui->analyzeButton, SIGNAL(pressed()), SLOT(analyze()));
    QObject::connect(ui->saveButton, SIGNAL(pressed()), SLOT(save()));
}

FsyncWindow::~FsyncWindow() {
    if (root)
        delete root;
    delete timer;
    delete ui;
}

void FsyncWindow::browseSourceFolder() {
    disableUi();
    browseFolder(*(ui->sourceEdit), "Select the source folder");
    enableUi();
}

void FsyncWindow::browseSaveFolder() {
    disableUi();
    browseFolder(*(ui->saveEdit), "Select the destination folder");
    enableUi();
}

void FsyncWindow::analyze() {
    cancel = false;
    disableUi();
    QDir srcDir(ui->sourceEdit->text());
    QDir dstDir(ui->saveEdit->text());

    if (!srcDir.exists() || !srcDir.isAbsolute()) {
        QMessageBox::critical(this, "Error", "The source folder path is invalid.");
        enableUi();
        return;
    }

    if (!dstDir.exists() || !dstDir.isAbsolute()) {
        QMessageBox::critical(this, "Error", "The destination folder path is invalid");
        enableUi();
        return;
    }

    resetUi();

    if (root)
        delete root;

    root = new Ftree(srcDir, dstDir);
    AnalyzeWorker* worker = new AnalyzeWorker(root);
    QObject::connect(worker, SIGNAL(itemChanged(QString)), SLOT(setCurrentItem(const QString&)));
    QObject::connect(worker, SIGNAL(finished()), SLOT(endAnalyze()));
    QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));

    QObject::disconnect(ui->analyzeButton, SIGNAL(pressed()), this, SLOT(analyze()));
    QObject::connect(ui->analyzeButton, SIGNAL(pressed()), SLOT(cancelAnalyze()));
    QObject::connect(ui->analyzeButton, SIGNAL(pressed()), worker, SLOT(cancelWork()));

    QObject::connect(timer, SIGNAL(timeout()), SLOT(updateTime()));
    time = -1;
    updateTime();

    ui->progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->timeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    worker->start();
    timer->start(1000);

    ui->analyzeButton->setText("Cancel");
    ui->analyzeButton->setEnabled(true);
}

void FsyncWindow::endAnalyze() {
    timer->stop();
    QObject::disconnect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    if (!cancel) {
        bool changes = root->getChangeCount() > 0;

        if (changes) {
            createTable(root);
        } else {
            ui->diffTable->setRowCount(1);
            ui->diffTable->setItem(0, 0, new QTableWidgetItem("No difference!"));
        }

        ui->progressBar->setValue(100);
        ui->itemLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        QMessageBox::information(this, "Analysis", "Analysis finished!");

        if (changes)
            ui->saveButton->setEnabled(true);
    }

    QObject::connect(ui->analyzeButton, SIGNAL(pressed()), SLOT(analyze()));
    QObject::disconnect(ui->analyzeButton, SIGNAL(pressed()), this, SLOT(cancelAnalyze()));

    ui->analyzeButton->setText("Start analysis");
    enableUi();
}

void FsyncWindow::save() {
    cancel = false;
    disableUi();
    ui->progressBar->setValue(0);

    ApplyWorker* worker = new ApplyWorker(root);
    QObject::connect(worker, SIGNAL(progressed()), SLOT(incrProgress()));
    QObject::connect(worker, SIGNAL(itemChanged(QString)), SLOT(setCurrentItem(const QString&)));
    QObject::connect(worker, SIGNAL(finished()), SLOT(endSave()));
    QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));

    QObject::disconnect(ui->saveButton, SIGNAL(pressed()), this, SLOT(save()));
    QObject::connect(ui->saveButton, SIGNAL(pressed()), SLOT(cancelSave()));
    QObject::connect(ui->saveButton, SIGNAL(pressed()), worker, SLOT(cancelWork()));

    QObject::connect(timer, SIGNAL(timeout()), SLOT(updateTime()));
    time = -1;
    updateTime();

    ui->progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->timeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->progressBar->setMaximum(root->getChangeCount());
    worker->start();
    timer->start(1000);

    ui->saveButton->setText("Cancel");
    ui->saveButton->setEnabled(true);
}

void FsyncWindow::endSave() {
    timer->stop();
    QObject::disconnect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    if (!cancel) {
        ui->progressBar->setValue(ui->progressBar->maximum());
        ui->itemLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        QMessageBox::information(this, "Back-up", "Back-up finished!");
    } else {
        QMessageBox::information(this, "Back-up", "Back-up canceled, please restart an analysis");
    }

    QObject::connect(ui->saveButton, SIGNAL(pressed()), SLOT(save()));
    QObject::disconnect(ui->saveButton, SIGNAL(pressed()), this, SLOT(cancelSave()));

    ui->saveButton->setText("Start back-up");
    ui->saveButton->setDisabled(true);
    enableUi();
}

void FsyncWindow::cancelAnalyze() {
    ui->analyzeButton->setDisabled(true);
    ui->analyzeButton->setText("Canceling...");
    cancel = true;
}

void FsyncWindow::cancelSave() {
    ui->saveButton->setDisabled(true);
    ui->saveButton->setText("Canceling...");
    cancel = true;
}

void FsyncWindow::incrProgress() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void FsyncWindow::setCurrentItem(const QString& text) {
    ui->itemLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->itemLabel->setText(text);
}

void FsyncWindow::disableUi() {
    ui->analyzeButton->setDisabled(true);
    ui->saveButton->setDisabled(true);
    ui->sourceBrowse->setDisabled(true);
    ui->saveBrowse->setDisabled(true);
    ui->settingsTab->setDisabled(true);
}

void FsyncWindow::enableUi() {
    ui->analyzeButton->setEnabled(true);
    ui->sourceBrowse->setEnabled(true);
    ui->saveBrowse->setEnabled(true);
    ui->settingsTab->setEnabled(true);
}

void FsyncWindow::updateTime() {
    int m, s;

    ++time;
    m = time/60;
    s = time%60;

    ui->timeLabel->setText(QString("Elapsed time: ") +
                           QString::number(m) + QString(":") +
                           (s < 10 ? QString("0") + QString::number(s) : QString::number(s)));
}

void FsyncWindow::resetUi() {
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);

    ui->diffTable->clear();
    ui->diffTable->setRowCount(0);
    ui->diffTable->setColumnCount(3);
    ui->diffTable->setHorizontalHeaderLabels(QStringList() << "" << "Source" << "Destination");

    ui->saveButton->setDisabled(true);
    ui->itemLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    ui->progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    ui->timeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
}

void FsyncWindow::browseFolder(QLineEdit& line, const char* msg) {
    QDir defaultDir(line.text());
    QString dir;

    if (!defaultDir.exists() || !defaultDir.isAbsolute())
        defaultDir = QDir::home();

    dir = QFileDialog::getExistingDirectory(this,
                                            QString::fromUtf8(msg),
                                            defaultDir.absolutePath(),
                                            QFileDialog::ShowDirsOnly
                                            | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty() && !dir.isNull()) {
        if (dir != line.text())
            resetUi();

        line.setText(dir);
    }
}

void FsyncWindow::addRow(QTableWidgetItem* t1, QTableWidgetItem* t2, QTableWidgetItem* t3) {
    int rowCount = ui->diffTable->rowCount();

    ui->diffTable->setRowCount(rowCount + 1);

    if (t1)
       ui->diffTable->setItem(rowCount, 0, t1);
    if (t2)
       ui->diffTable->setItem(rowCount, 1, t2);
    if (t3)
       ui->diffTable->setItem(rowCount, 2, t3);
}

void FsyncWindow::createTable(Ftree* tree) {
    QTableWidgetItem *signItem, *textItem;

    for (auto it = tree->getDirList()->begin(); it != tree->getDirList()->end(); ++it) {
        signItem = new QTableWidgetItem("+d");
        signItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        textItem = new QTableWidgetItem(it->absoluteFilePath());
        textItem->setBackgroundColor(Qt::green);

        addRow(signItem, textItem, nullptr);
    }

    for (auto it = tree->getFileList()->begin(); it != tree->getFileList()->end(); ++it) {
        signItem = new QTableWidgetItem("+f");
        signItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        textItem = new QTableWidgetItem(it->absoluteFilePath());
        textItem->setBackgroundColor(Qt::green);

        addRow(signItem, textItem, nullptr);
    }

    for (auto it = tree->getRemList()->begin(); it != tree->getRemList()->end(); ++it) {
        signItem = new QTableWidgetItem(it->isDir() ? "-d" : "-f");
        signItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        textItem = new QTableWidgetItem(it->absoluteFilePath());
        textItem->setBackgroundColor(Qt::red);

        addRow(signItem, nullptr, textItem);
    }

    for (auto it = tree->getChildren()->begin(); it != tree->getChildren()->end(); ++it)
        createTable(*it);
}
