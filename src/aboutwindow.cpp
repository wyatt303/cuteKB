#include "aboutwindow.h"
#include "ui_aboutwindow.h"

static const QString COPYRIGHT = QStringLiteral("Copyleft 2023 wyatt303");
static const QString HOMEPAGE = QStringLiteral("");

aboutwindow::aboutwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutwindow)
{
    ui->setupUi(this);
    ui->messageLabel->setText(QString("<p><b><font size=\"5\">cuteKB</font></b><br>Version %1</p>").arg(qApp->applicationVersion()));

    const QString description = QString("<p>%1<br>%2</p><p>%3</p>")
            .arg(tr("Qt-based, free and open source markdown editor with live HTML preview"))
            .arg(COPYRIGHT)
            .arg(HOMEPAGE);
   // ui->descriptionLabel->setText(description);
}

aboutwindow::~aboutwindow()
{
    delete ui;
}
