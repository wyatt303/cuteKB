#include "editorwindow.h"
#include "ui_editorwindow.h"

editorWindow::editorWindow(QWidget *parent, QSettings *mysettings, QSqlDatabase *db, int row) :
    QMainWindow(parent),
    ui(new Ui::editorWindow)
{
    ui->setupUi(this);
    currentRow = row;
    currCon = db;
    topicEdit = new QLineEdit(ui->toolBar);
    comboGroups = new QComboBox();
    ui->toolBar->addWidget(topicEdit);
    ui->toolBar->addWidget(comboGroups);

    mysettings->beginGroup("Http");
    hostUrl = mysettings->value("Host").toString();
    scriptUrl = mysettings->value("Script").toString();
    mysettings->endGroup();
    mysettings->beginGroup("UserCredentials");
    userName = mysettings->value("UserName").toString();
    mysettings->endGroup();
    now = now.currentDateTime();
    if (currCon != NULL) {
        // fill combobox with group names

        modelGroups = new QSqlTableModel(this, *currCon);
        modelGroups->setTable("Groups");

        //modelGroups->setEditStrategy(QSqlTableModel::OnFieldChange);
        modelGroups->select();
        comboGroups->setModel(modelGroups);
        comboGroups->setModelColumn(1);
        connect(comboGroups, SIGNAL(currentIndexChanged(int)), this, SLOT(setRightGroup(int)));
        query = new QSqlQuery(*currCon);
        if (row == 0) {
            groupID = 1;
        } else {


            query->prepare("SELECT contentData, contentTopic, groupId FROM Content WHERE idContent=:row");
            query->bindValue(":row", row);
            query->exec();
            //qDebug() << query->lastError();
            //qDebug() << query->lastQuery();
            query->first();
            ui->textEdit->setText(query->value(0).toString());
            topicEdit->setText(query->value(1).toString());
            groupID = query->value(2).toInt();
            int i = 0;
            while (groupID != modelGroups->data(modelGroups->index(comboGroups->currentIndex(),0)).toInt()) {
                qDebug() << "id:  " << query->value(2).toInt();
                qDebug() << "id2: " << modelGroups->data(modelGroups->index(comboGroups->currentIndex(),0)).toInt();
                comboGroups->setCurrentIndex(i);
                i++;
            }

            //comboGroups->setCurrentIndex(i);
            //qDebug() << row;
            query->finish();


         }
      } else {
        //close window
    }
    ui->textEdit->setFocus();

}

editorWindow::~editorWindow()
{
    currCon = 0;
    delete ui;
}

void editorWindow::on_actionSave_triggered()
{
    if (currentRow != 0) {
        query->prepare("UPDATE Content SET contentData=:data, groupID=:group, contentTopic=:topic, contentAuthor=:author, contentUpdate=:date WHERE idContent=:row");
        query->bindValue(":group", groupID);
        query->bindValue(":data", ui->textEdit->toPlainText());
        query->bindValue(":topic", topicEdit->text());
        query->bindValue(":row", currentRow);
        query->bindValue(":date", now);
        query->exec();
        //qDebug() << query->lastQuery();
        //qDebug() << query->lastError();
        query->finish();
    } else {
        query->prepare("INSERT INTO Content (contentData, contentTopic, contentAuthor, contentDate, groupId) VALUES(:data, :topic, :author, :date, :group)");
        query->bindValue(":group", groupID);
        query->bindValue(":data", ui->textEdit->toPlainText());
        query->bindValue(":topic", topicEdit->text());
        query->bindValue(":author", userName);
        query->bindValue(":date", now);
        query->exec();
        query->finish();
    }
}

void editorWindow::setRightGroup(int id) {
     groupID = modelGroups->data(modelGroups->index(comboGroups->currentIndex(),0)).toInt();
     qDebug() << groupID;
}

void editorWindow::on_actionBold_triggered()
{
    QTextCursor c = ui->textEdit->textCursor();
    QString selectedText = ui->textEdit->textCursor().selectedText();

    if (selectedText.isEmpty()) {
        c.insertText("****");
        c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        ui->textEdit->setTextCursor(c);
    } else {
        c.insertText("**" + selectedText + "**");
    }
}

void editorWindow::on_actionAddImage_triggered()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("Image files (*.jpg *.png *.gif *.jpeg)"));
    dialog.setWindowTitle(tr("Select image to insert"));
    if (dialog.exec() == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            QNetworkRequest request(QUrl(hostUrl + scriptUrl + "upload.php")); //our server with php-script
            file.setFileName(fileNames.at(0));
            if (!file.open(QIODevice::ReadOnly))
                return;
            QString bound="margin"; //name of the boundary
            //according to rfc 1867 we need to put this string here:
            QByteArray data(QString("--" + bound + "\r\n").toLocal8Bit());
            data.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
            data.append("upload.php\r\n");
            data.append("--" + bound + "\r\n"); //according to rfc 1867
            data.append("Content-Disposition: form-data; name=\"image\"; filename=\""+ file.fileName() +"\"\r\n"); //name of the input is "uploaded" in my form, next one is a file name.
            data.append("Content-Type: image/jpeg\r\n\r\n"); //data type
            data.append(file.readAll()); //let's read the file
            data.append("\r\n");
            data.append("--" + bound + "--\r\n"); //closing boundary according to rfc 1867
            request.setRawHeader(QString("Content-Type").toLocal8Bit(),QString("multipart/form-data; boundary=" + bound).toLocal8Bit());
            request.setRawHeader(QString("Content-Length").toLocal8Bit(), QString::number(data.length()).toLocal8Bit());
            connect(&am, SIGNAL(finished(QNetworkReply*)), this, SLOT(replayFinished(QNetworkReply*)));
            this->reply = am.post(request,data);
            fileName = file.fileName();
            qDebug() << data.data();


        }
    }
}

void editorWindow::replayFinished(QNetworkReply *reply)
{
    QTextCursor c = ui->textEdit->textCursor();
    QFileInfo fileInfo(file.fileName());
    QByteArray ba = reply->readAll();
    QString myString(ba);
//    myString = myString.left(3);
    QMessageBox msgBox;
    if (myString.left(3) == "Ok ") {
        myString = myString.mid(3,myString.indexOf("<")-3);
        if (myString == "jpeg" || myString == "jpg" || myString == "png" || myString == "gif") {
            c.insertText("!["+ fileInfo.baseName() + "]("+ hostUrl + scriptUrl +"images/" + fileInfo.baseName()+"."+fileInfo.suffix()+")\n");
        } else {
            c.insertText("["+ fileInfo.baseName() + "]("+ hostUrl + scriptUrl +"images/" + fileInfo.baseName()+"."+fileInfo.suffix()+")\n");
        }
        msgBox.setText("File was uploaded");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
    if (myString.left(3) == "Exi") {
        c.insertText("!["+ fileInfo.baseName() + "]("+ hostUrl + scriptUrl +"images/" + fileInfo.baseName()+"."+fileInfo.suffix()+ ")\n");
        msgBox.setText("File already exists");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
    if (myString.left(3) == "Siz") {
        msgBox.setText("File to large - Limit 4 MB");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    if (myString.left(3) == "Ext") {
        msgBox.setText("Wrong file extension");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        // allowed extansion set in upload.php script
        // "jpeg","jpg","png","gif","pdf","doc","docx","xls","xlsx"
    }

    qDebug() << myString;
}

void editorWindow::on_actionItalic_triggered()
{
    QTextCursor c = ui->textEdit->textCursor();
    QString selectedText = ui->textEdit->textCursor().selectedText();

    if (selectedText.isEmpty()) {
        c.insertText("**");
        c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        ui->textEdit->setTextCursor(c);
    } else {
        c.insertText("*" + selectedText + "*");
    }
}

void editorWindow::on_actionCode_triggered()
{
    QTextCursor c = ui->textEdit->textCursor();
    QString selectedText = ui->textEdit->textCursor().selectedText();

    if (selectedText.isEmpty()) {
        c.insertText("``");
        c.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        ui->textEdit->setTextCursor(c);
    } else {
        c.insertText("`" + selectedText + "`");
    }
}

void editorWindow::on_actionTable_triggered()
{
    QTextCursor c = ui->textEdit->textCursor();
    c.insertText("|   |   |\n|---|---|\n|   |   |");
}

void editorWindow::on_actionAddFile_triggered()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("Document files (*.pdf *.doc *.docx *.xls *.xlsx *.sh *.ods *.odt *.zip)"));
    dialog.setWindowTitle(tr("Select document to insert"));
    if (dialog.exec() == QDialog::Accepted) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.count() > 0) {
            QNetworkRequest request(QUrl(hostUrl + scriptUrl + "upload.php")); //our server with php-script
            file.setFileName(fileNames.at(0));
            if (!file.open(QIODevice::ReadOnly))
                return;
            QString bound="margin"; //name of the boundary
            //according to rfc 1867 we need to put this string here:
            QByteArray data(QString("--" + bound + "\r\n").toLocal8Bit());
            data.append("Content-Disposition: form-data; name=\"action\"\r\n\r\n");
            data.append("upload.php\r\n");
            data.append("--" + bound + "\r\n"); //according to rfc 1867
            data.append("Content-Disposition: form-data; name=\"image\"; filename=\""+ file.fileName() +"\"\r\n"); //name of the input is "uploaded" in my form, next one is a file name.
            data.append("Content-Type: image/jpeg\r\n\r\n"); //data type
            data.append(file.readAll()); //let's read the file
            data.append("\r\n");
            data.append("--" + bound + "--\r\n"); //closing boundary according to rfc 1867
            request.setRawHeader(QString("Content-Type").toLocal8Bit(),QString("multipart/form-data; boundary=" + bound).toLocal8Bit());
            request.setRawHeader(QString("Content-Length").toLocal8Bit(), QString::number(data.length()).toLocal8Bit());
            this->reply = am.post(request,data);
            fileName = file.fileName();
            qDebug() << data.data();
            connect(&am, SIGNAL(finished(QNetworkReply*)), this, SLOT(replayFinished(QNetworkReply*)));

        }
    }
}
