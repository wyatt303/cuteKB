#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QtWidgets>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QtWidgets>
#include <QCoreApplication>
#include <QSql>
#include <QDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>
#include <QTextEdit>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>

namespace Ui {
class editorWindow;
}

class editorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit editorWindow(QWidget *parent = 0, QSettings *mysettings = 0, QSqlDatabase *db = 0, int row = 0);
    ~editorWindow();

private slots:
    void on_actionSave_triggered();
    void setRightGroup(int);

    void on_actionBold_triggered();

    void on_actionAddImage_triggered();
    void replayFinished(QNetworkReply *reply);

    void on_actionItalic_triggered();

    void on_actionCode_triggered();

    void on_actionTable_triggered();

    void on_actionAddFile_triggered();

private:
    Ui::editorWindow *ui;
    QSqlDatabase *currCon;
    QSqlQuery 	*query;
    int 		currentRow, groupID;
    int 		z;
    QComboBox   *comboGroups;
    QString      hostUrl, scriptUrl, userName;
    QSettings *settings;
    QLineEdit	*topicEdit;

    QSqlTableModel *modelGroups;
    QNetworkAccessManager am;
    QNetworkReply *reply;
    QString fileName;
    QFile file;
    QDateTime now;
};

#endif // EDITORWINDOW_H
