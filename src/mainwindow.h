#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QtWidgets>
#include <QCoreApplication>
#include <QSql>
#include <QMainWindow>
#include <QSettings>
#include <QDialog>
#include <QLineEdit>
#include <QImage>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineDownloadItem>
#include "settingswindow.h"
#include "editorwindow.h"
#include "aboutwindow.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSqlDatabase 	MySQL, SQLite, currCon;
    QSettings settings;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionNewentry_triggered();
    void on_actionSettings_triggered();
    void on_actionRefresh_triggered();
    void Refresh();
    void readSelected();
    void tocLinkClicked(const QUrl &url);
    void searchInDB(QString);
    void styleGithub();
    void downloadRequested(QWebEngineDownloadItem*);

    void on_actionEditentry_triggered();

    void on_actionFindPrevious_triggered();

    void on_actionFindNext_triggered();

    void on_actionDeleteEntry_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    settingsWindow *settingsDialog;
    editorWindow *editorDialog;
    aboutwindow *aboutDialog;

    bool openSettingsdialog();
    QLineEdit *searchEdit;
    // Query
    QSqlQuery 		*query;
    QSqlQueryModel 	*qModel;
    QItemSelectionModel *selectionModel;
    int mode;


    QLabel *connLabel;

    QString markdownToHTML(QString);
    QString markdownToToC(QString);

};

#endif // MAINWINDOW_H
