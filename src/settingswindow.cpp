#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <QFileDialog>
#include <QSettings>

settingsWindow::settingsWindow(QWidget *parent, QSettings *mysettings, QSqlDatabase *MySQL) :
    QDialog(parent),
    ui(new Ui::settingsWindow)
{
    ui->setupUi(this);
    settings = mysettings;
    remoteConnection = MySQL;
    if (settings->value("Database").toString() == "local") {
        ui->groupBoxSQLite->setChecked(true);
        ui->groupBoxMySQL->setChecked(false);
    } else {
        ui->groupBoxSQLite->setChecked(false);
        ui->groupBoxMySQL->setChecked(true);
    }
    settings->beginGroup("LocalDatabase");
    ui->pathTextEdit->setText(settings->value("Database").toString());
    settings->endGroup();
    settings->beginGroup("RemoteDatabase");
    ui->userTextEdit->setText(settings->value("User").toString());
    ui->passwordTextEdit->setText(settings->value("Password").toString());
    ui->databaseTextEdit->setText(settings->value("Database").toString());
    ui->hostTextEdit->setText(settings->value("Host").toString());
    ui->portTextEdit->setText(settings->value("Port").toString());
    settings->endGroup();
    for(int row = 0; row < ui->listWidget->count(); row++)
    {
       ui->listWidget->item(row)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
      //  item->setTextAlignment();
    }
}

settingsWindow::~settingsWindow()
{
    settings = 0;
    remoteConnection = 0;
    delete ui;
}

void settingsWindow::on_buttonBox_accepted()
{
    if (ui->groupBoxSQLite->isChecked()) {
        settings->setValue("Database", "local");
    } else {
        settings->setValue("Database", "remote");
    }
    settings->beginGroup("LocalDatabase");
    settings->setValue("Database", ui->pathTextEdit->text());
    settings->endGroup();
    settings->beginGroup("RemoteDatabase");
    settings->setValue("User", ui->userTextEdit->text());
    settings->setValue("Password", ui->passwordTextEdit->text());
    settings->setValue("Database", ui->databaseTextEdit->text());
    settings->setValue("Host", ui->hostTextEdit->text());
    settings->setValue("Port", ui->portTextEdit->text());
    settings->endGroup();
    setResult(QDialog::Accepted);
}

void settingsWindow::on_buttonBox_rejected()
{
    setResult(QDialog::Rejected);
}

void settingsWindow::on_testButton_clicked()
{
    QMessageBox msgBox;
    remoteConnection->setHostName(ui->hostTextEdit->text());
    remoteConnection->setDatabaseName(ui->databaseTextEdit->text());
    remoteConnection->setUserName(ui->userTextEdit->text());
    remoteConnection->setPassword(ui->passwordTextEdit->text());
    remoteConnection->setPort(ui->portTextEdit->text().toInt());
    remoteConnection->setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=5");
    if (remoteConnection->open()) {
        msgBox.setText("Connection completed successfully");
        msgBox.exec();
    } else {
        msgBox.setText(remoteConnection->lastError().text());
        msgBox.exec();
    }

}

void settingsWindow::on_listWidget_clicked(const QModelIndex &index)
{
    ui->stackedWidget->setCurrentIndex(index.row());
}

void settingsWindow::on_pathButton_clicked()
{
    const QFileDialog::Options options = QFlag(QFileDialog::ReadOnly);
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                ui->pathTextEdit->text(),
                                tr("*.sqlite"),
                                tr("All SQLite files (*.sqlite);; All files (*);;"),
                                &selectedFilter,
                                options);
    if (!fileName.isEmpty()) {
        ui->pathTextEdit->setText(fileName);
    }
}


void settingsWindow::on_groupBoxMySQL_clicked()
{
    ui->groupBoxSQLite->setChecked(false);
}


void settingsWindow::on_groupBoxSQLite_clicked()
{
    ui->groupBoxMySQL->setChecked(false);
}

