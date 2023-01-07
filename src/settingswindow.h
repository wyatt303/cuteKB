#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QtSql>
#include <QMessageBox>

namespace Ui {
class settingsWindow;
}

class settingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit settingsWindow(QWidget *parent = 0, QSettings *mysettings = 0,  QSqlDatabase *MySQL = 0);
    ~settingsWindow();
    QSqlTableModel *modelGroups;

private:
    Ui::settingsWindow *ui;
    QSettings *settings;
    QSqlDatabase *remoteConnection;


private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_testButton_clicked();
    void on_listWidget_clicked(const QModelIndex &index);
    void on_pathButton_clicked();
    void on_groupBoxMySQL_clicked();
    void on_groupBoxSQLite_clicked();
};

#endif // SETTINGSDIALOG_H
