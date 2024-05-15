#include "properties.h"
#include <QFileDialog>
#include <QDebug>
#include <QSettings>

Properties::Properties(QWidget *parent) : QDialog(parent)
{

    width = 600;
    height = 400;
    pathToTex = "";  // Will later be set by the value from the ini file.

    texLine = new QLineEdit(pathToTex, this);
    texLab = new QLabel(tr("Latex: "), this);
    timeOutLab = new QLabel(tr("Latex Timeout [ms]: "), this);
    timeOut = new QSpinBox(this);
    lay = new QGridLayout(this);
    save = new QPushButton(tr("Save"), this);
    discard=new QPushButton(tr("Discard"), this);
    selectFolder = new QPushButton(tr("Select"), this);
    dialButBox = new QDialogButtonBox(this);

    dialButBox->addButton(save, QDialogButtonBox::AcceptRole);
    dialButBox->addButton(discard, QDialogButtonBox::RejectRole);

    timeOut->setRange(1000, 30000);
    texLine->setFixedWidth(300);

    lay->addWidget(texLab,      1,1,1,1);
    lay->addWidget(texLine,     1,2,1,2);
    lay->addWidget(selectFolder,1,4,1,1);
    lay->addWidget(timeOutLab,  2,1,1,2);
    lay->addWidget(timeOut,     2,3,1,1);
    lay->addWidget(dialButBox,  3,2,1,2);
    this->setLayout(lay);

    connect(selectFolder,SIGNAL(clicked(bool)),this,SLOT(pathDialog()));
    connect(save,SIGNAL(clicked(bool)),this,SLOT(saveProperties()));
    connect(discard,SIGNAL(clicked(bool)),this,SLOT(close()));

    loadProperties();

}

void Properties::pathDialog() {
    // Get the path to latex.exe
    QString path = QFileDialog::getExistingDirectory(nullptr,
        tr("Directory that contains Latex.exe and Dvipng.exe"), pathToTex);

    if(!path.isEmpty()) {
        this->texLine->setText(path);
    }
}

void Properties::saveProperties(){
    //Save properties to saveFileName
    QSettings settings(saveFileName,QSettings::IniFormat);
    settings.setValue("LatexPath", texLine->text());
    settings.setValue("Timeout", timeOut->value());
    this->pathToTex = texLine->text();
    this->time = timeOut->value();
    this->close();
    emit accepted();
}

void Properties::loadProperties() {
    // Load properties from saveFileName
    QSettings settings(saveFileName, QSettings::IniFormat);
    texLine->setText(settings.value("LatexPath").toString());
    this->pathToTex = texLine->text();
    timeOut->setValue(settings.value("Timeout").toInt());
    this->time = timeOut->value();
}

