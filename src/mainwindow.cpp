#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "library/hoedown/html.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mode = 0;
    if (settings.value("Database").toString() == "local") {
        SQLite = QSqlDatabase::addDatabase("QSQLITE", "mySQLite" );
        settings.beginGroup("LocalDatabase");
        if (settings.value("Database").toString() != NULL) {
           SQLite.setDatabaseName(settings.value("Database").toString());
        } else {
            if (openSettingsdialog()) {
                SQLite.setDatabaseName(settings.value("Database").toString());
            }
        }
        settings.endGroup();
        if(!SQLite.open()) {
            qDebug() << SQLite.lastError();
            //qFatal( "Failed to connect." );
            QPixmap connPixmap(":/images/ico/disconnected.png");
            connLabel = new QLabel();
            connLabel->setPixmap(connPixmap);
            ui->statusBar->addWidget(connLabel);
        } else {
            QPixmap connPixmap(":/images/ico/connected.png");
            connLabel = new QLabel();
            connLabel->setPixmap(connPixmap);
            ui->statusBar->addWidget(connLabel);
            currCon = SQLite;
        }
    } else {
        MySQL = QSqlDatabase::addDatabase("QMYSQL", "myMySQL");
        settings.beginGroup("RemoteDatabase");
        if (!MySQL.isOpen()) {
            if (settings.value("Database").toString() != NULL) {
                MySQL.setUserName(settings.value("User").toString());
                MySQL.setPassword(settings.value("Password").toString());
                MySQL.setDatabaseName(settings.value("Database").toString());
                MySQL.setHostName(settings.value("Host").toString());
                MySQL.setPort(settings.value("Port").toInt());
                if (MySQL.open()) {
                    QPixmap connPixmap(":/images/ico/connected.png");
                    connLabel = new QLabel();
                    connLabel->setPixmap(connPixmap);
                    ui->statusBar->addWidget(connLabel);
                    currCon = MySQL;
                } else {
                    QPixmap connPixmap(":/images/ico/disconnected.png");
                    connLabel = new QLabel();
                    connLabel->setPixmap(connPixmap);
                    ui->statusBar->addWidget(connLabel);
                }
            }
        }
        settings.endGroup();
    }

    // **************************************************************
    // *                        SQL Queries                         *
    // **************************************************************



    query = new QSqlQuery(currCon);
    qModel = new QSqlQueryModel();

    qModel->setQuery("SELECT * FROM Main", currCon);
    if (qModel->lastError().isValid()) {
        qDebug() << qModel->lastError();
    }

    qModel->setHeaderData(1, Qt::Horizontal, tr("Topic"));
    qModel->setHeaderData(3, Qt::Horizontal, tr("Group"));


    ui->tableView->setModel(qModel);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QTableView::SingleSelection);
    ui->tableView->hideColumn(0);
    ui->tableView->showColumn(1);
    ui->tableView->hideColumn(2);
    ui->tableView->showColumn(3);
    ui->tableView->resizeColumnsToContents();

    ui->tableView->show();
    selectionModel=ui->tableView->selectionModel();
    connect(selectionModel, SIGNAL(selectionChanged(QItemSelection ,QItemSelection)), this , SLOT(readSelected()));
    ui->tableView->selectRow(0);

    searchEdit = new QLineEdit;
    //ui->mainToolBar->addWidget(searchEdit);
    ui->mainToolBar->insertWidget(ui->actionFindPrevious, searchEdit);
    connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchInDB(QString)));
    connect(QWebEngineProfile::defaultProfile(), SIGNAL(downloadRequested(QWebEngineDownloadItem*)),
                    this, SLOT(downloadRequested(QWebEngineDownloadItem*)));
    //styleGithub();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openSettingsdialog()
{
    settingsWindow *settingsDialog = new settingsWindow(this, &settings, &MySQL);
    if (settingsDialog->exec() == QDialog::Accepted) {
        //save & reload settings
        settings.sync();
        return true;
    } else {
        // do nothing
        return false;
    }
}

void MainWindow::Refresh()
{
    qModel->setQuery("SELECT * FROM Main", currCon);
    if (qModel->lastError().isValid()) {
        qDebug() << qModel->lastError();
        ui->actionDeleteEntry->setEnabled(false);
    }

}

void MainWindow::readSelected()
{
    QString mark;
    mark = qModel->data(qModel->index(selectionModel->currentIndex().row(),2)).toString();
    mode = qModel->data(qModel->index(selectionModel->currentIndex().row(),0)).toInt();

    ui->textBrowser->setHtml(markdownToHTML(mark));
    //ui->textBrowser->setText(markdownToHTML(mark));
    ui->textBrowserTOC->setHtml(markdownToToC(mark));

    ui->actionDeleteEntry->setEnabled(true);
    qDebug() << mode;
}

QString MainWindow::markdownToToC(QString str)
{
    hoedown_renderer *renderer =  hoedown_html_toc_renderer_new( 16);
    hoedown_extensions extensions = (hoedown_extensions) (HOEDOWN_EXT_BLOCK | HOEDOWN_EXT_SPAN);
    hoedown_document *document = hoedown_document_new(renderer, extensions, 16);

    unsigned char *sequence = (unsigned char *) qstrdup(str.toUtf8().constData());
    int length = strlen((char *) sequence);

    // return an empty string if the note is empty
    if (length == 0) {
        return "";
    }

    hoedown_buffer *html = hoedown_buffer_new(length);

    // render markdown toc
    hoedown_document_render(document, html, sequence, length);

    // get markdown toc
    QString result = QString::fromUtf8((char *) html->data, html->size);

    /* Cleanup */
    free(sequence);
    hoedown_buffer_free(html);

    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);

    result = "<html><head>\n<style type=\"text/css\">ul { list-style-type: none; padding: 20px; margin-left: -2em; } a { text-decoration: none; } p { background-color: #222222; } table {bgcolor: #222222; }</style>\n</head><body>"
            + result + "</body></html>";
    return result;
}

QString MainWindow::markdownToHTML(QString str)
{
        // HOEDOWN_HTML_USE_XHTML
    hoedown_renderer *renderer =  hoedown_html_renderer_new(HOEDOWN_HTML_HARD_WRAP, 16);
    hoedown_extensions extensions = (hoedown_extensions) (HOEDOWN_EXT_BLOCK | HOEDOWN_EXT_SPAN);
    hoedown_document *document = hoedown_document_new(renderer, extensions, 16);

    // parse for relative file urls and make them absolute
    // (for example to show images under the note path)
//    str.replace(
//            QRegularExpression("\\(file:\\/\\/([^\\/].+)\\)"),
//            "(file://" + notesPath + "/\\1)");

    unsigned char *sequence = (unsigned char *) qstrdup(str.toUtf8().constData());
    int length = strlen((char *) sequence);

    // return an empty string if the note is empty
    if (length == 0) {
        return "";
    }

    hoedown_buffer *html = hoedown_buffer_new(length);

    // render markdown html
    hoedown_document_render(document, html, sequence, length);

    // get markdown html
    QString result = QString::fromUtf8((char *) html->data, html->size);

    /* Cleanup */
    free(sequence);
    hoedown_buffer_free(html);

    hoedown_document_free(document);
    hoedown_html_renderer_free(renderer);

    result =
            "<html><head><style>h1, h2, h3 { margin: 2pt 0 5pt 0; }"
            "code {display:block; background-color: #ecf9df; border: 1px solid; border-color: #c8e2b8; } a { color: #000000; text-decoration: none; } table {padding: 0;border-collapse: collapse;} table tr {border-top: 1px solid #cccccc; background-color: white; margin: 0; padding: 0; } table tr:nth-child(2n) {background-color: #f8f8f8; } table tr th {font-weight: bold; border: 1px solid #cccccc; margin: 0; padding: 6px 13px; } table tr td {border: 1px solid #cccccc; margin: 0; padding: 6px 13px; } table tr th :first-child, table tr td :first-child { margin-top: 0; } table tr th :last-child, table tr td :last-child {margin-bottom: 0; }"
            "</style></head><body>" +
            result + "</body></html>";

    qDebug() << result;
    // check if width of embedded local images is too high
    QRegularExpression re("<img src=\"http:\\/\\/([^\"]+)\"");
    QRegularExpressionMatchIterator i = re.globalMatch(result);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString fileName = match.captured(1);
        QImage image(fileName);

        // cap the image width at 980px
        if (image.width() > 980) {
            result.replace(
                    QRegularExpression("<img src=\"http:\\/\\/" + QRegularExpression::escape(fileName) + "\""),
                           "<img width=\"980\" src=\"http://" + fileName + "\"");
        }
    }
    return result;
    //return str;
}

void MainWindow::on_actionSettings_triggered()
{
    openSettingsdialog();
}

void MainWindow::on_actionRefresh_triggered()
{
    Refresh();
}

void MainWindow::tocLinkClicked(const QUrl &url)
{
    QString anchor = url.toString().remove("#");
 //   QWebFrame frame();
 //   ui->textBrowser->page()->mainFrame()->scrollToAnchor(anchor);
    ui->textBrowser->page()->runJavaScript("var element_to_scroll_to = document.getElementById('" + anchor +"');element_to_scroll_to.scrollIntoView();");

}

void MainWindow::on_actionNewentry_triggered()
{
    editorDialog = new editorWindow(this, &settings, &currCon, 0);
    editorDialog->setWindowModality(Qt::ApplicationModal);
    editorDialog->setWindowFlags(Qt::Dialog);
    editorDialog->show();
}

void MainWindow::on_actionEditentry_triggered()
{
    editorDialog = new editorWindow(this, &settings, &currCon, mode);
    editorDialog->setWindowModality(Qt::ApplicationModal);
    editorDialog->setWindowFlags(Qt::Dialog);
    editorDialog->show();
}

void MainWindow::searchInDB(QString text) {
    if (text != NULL) {
        qModel->setQuery("SELECT * FROM Main WHERE contentData LIKE '%" + text + "%'", currCon);
        qDebug() << text;
        ui->tableView->selectRow(0);
        //ui->textBrowser->moveCursor(QTextCursor::Start);
        //ui->textBrowser->findText(searchEdit->text());
    searchEdit->setFocus();
    on_actionFindNext_triggered();
    if (qModel->lastError().isValid()) {
        qDebug() << qModel->lastError();
        }
    } else {

    }
}

void MainWindow::styleGithub() {
    //generator->setCodeHighlightingStyle("github");

    //ui->plainTextEdit->loadStyleFromStylesheet(":/theme/default.txt");
   // ui->textBrowser->settings()->setUserStyleSheetUrl(QUrl("qrc:/styles/css/github.css"));

    //styleLabel->setText(ui->actionGithub->text());
    //options->setLastUsedStyle(ui->actionGithub->objectName());
}

void MainWindow::downloadRequested(QWebEngineDownloadItem *download)
{
    QRegularExpression re("(?=\\w+\\.\\w{3,4}$).+"); // RegEx to get fine name from URL.
    QRegularExpressionMatch match = re.match(download->url().toString());
    if (match.hasMatch()) {
        QString filename = match.captured(0);
        filename = QFileDialog::getSaveFileName(this, "Save file", download->path());
        download->setPath(filename);
        download->accept();
    }
}

void MainWindow::on_actionFindPrevious_triggered()
{
    ui->textBrowser->findText(searchEdit->text(), QWebEnginePage::FindBackward);
}

void MainWindow::on_actionFindNext_triggered()
{
    ui->textBrowser->findText(searchEdit->text());
}

void MainWindow::on_actionDeleteEntry_triggered()
{
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::AutoText);
    msgBox.setText("Are you sure that you want to delete entry:\n" + qModel->data(qModel->index(selectionModel->currentIndex().row(),1)).toString() + "\nfrom Database");
    //msgBox.setInformativeText("Are you sure that you want to delete entry:\n" + qModel->data(qModel->index(selectionModel->currentIndex().row(),1)).toString() + "\n from Database");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Critical);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes) {

        query->prepare("DELETE FROM Content WHERE idContent=:row");
        query->bindValue(":row", mode);
        query->exec();
        query->finish();

        Refresh();
    }
    //    deleteAction->setEnabled(false);
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDialog = new aboutwindow(this);
    aboutDialog->setWindowModality(Qt::ApplicationModal);
    aboutDialog->setWindowFlags(Qt::Dialog);
    aboutDialog->show();
}
