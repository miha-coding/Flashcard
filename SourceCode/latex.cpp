#include "latex.h"
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QTextStream>
//#include <QTextCodec>
#include <memory>

#include <QDebug>

#define tr QObject::tr


Latex::Latex()
{
    pathToTex = " ";
    filePath = QDir::currentPath().append("/src/");
    time = 2000;

    notePix = QPixmap(getFilePath().append(tr("note.png")));
    questionPix = QPixmap(getFilePath().append(tr("question.png")));
    answerPix = QPixmap(getFilePath().append(tr("answer.png")));
    whitePix = QPixmap(getFilePath().append(tr("white.png")));

    if(!initStrings()) throw "Latex::initStrings not successfull!";
}

Latex::Latex(const QString texDirectory) {
    pathToTex = texDirectory;
    filePath = QDir::currentPath().append("/src/");
    time = 2000;

    notePix = QPixmap(getFilePath().append(tr("note.png")));
    questionPix = QPixmap(getFilePath().append(tr("question.png")));
    answerPix = QPixmap(getFilePath().append(tr("answer.png")));
    whitePix = QPixmap(getFilePath().append(tr("white.png")));

    if(!initStrings()) throw "Latex::initStrings not successfull!";
}

bool Latex::createTexFile(const QString content, const QString fileName) {
    // Writes the text content (from the DB) in the tex file.

    if (fileName!="note" && fileName!="question" && fileName!="answer") {
        QMessageBox::critical(0, tr("Create Tex-File"),
                              tr("Filename is not correct!"), QMessageBox::Ok);
        return false;
    }
    QFile file(QDir::currentPath().append("/src/").append(fileName).append(tr(".tex")));
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Fehler in createTexFile";
        return false;
    }
    QTextStream stream(&file);
    //stream.setCodec(QTextCodec::codecForName("UTF-8"));

    if (fileName=="question") {
        stream << firstPartQuestion << "%splitTexFile\n" << content.toUtf8() << "%splitTexFile" << lastPartQuestion;
        //qDebug() << firstPartQuestion;
    }
    else {
        stream << firstPart << "%splitTexFile\n" << content.toUtf8() << "%splitTexFile" << lastPart;
    }
    stream.reset();
    //qDebug() << "Content: " << content.toUtf8();
    file.close();
    return true;

}

bool Latex::initStrings() {
    // Read the existing .tex files and store the common texts in the appropriate
    // variables. The common text is e.g. the usepackages, begin{document}, etc.

    QString str;
    QFile file(QDir::currentPath().append("/src/note.tex"));
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, tr("Latex"),
                              tr("TexStrings could not be createt!\n If you use"
                                 " Latex, please try to restart this program"),
                              QMessageBox::Ok);
        return false;
    }

    QTextStream stream(&file);
    str = stream.readAll();
    QStringList strList = str.split(tr("%splitTexFile"));

    this->firstPart = strList.at(0);
    this->lastPart = strList.at(2);

    file.close();

    file.setFileName(QDir::currentPath().append("/src/question.tex"));
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, tr("Latex"),
                              tr("TexStrings could not be createt!\n If you use"
                                 " Latex, please try to restart this program"),
                              QMessageBox::Ok);
        return false;
    }

    stream.setDevice(&file);
    str = stream.readAll();

    strList.clear();
    strList = str.split(tr("%splitTexFile"));

    this->firstPartQuestion = strList.at(0);
    this->lastPartQuestion = strList.at(2);

    file.close();

    //qDebug() << "FirstPart:\n" << firstPart;
    //qDebug() << "SecondPart:\n" << lastPart;

    return true;
}

int Latex::texToPng(const QString fileName) {
    // Converts the tex file to png.

    if ((fileName!="note" && fileName!="question" && fileName!="answer")) {
        QMessageBox::critical(0,tr("Create Tex-File"),tr("Filename is not correct!"),QMessageBox::Ok);
        return -1;
    }

    // 1. Step: Convert .tex to .dvi
    QString latex = QString(pathToTex).append("/pdflatex.exe");
    auto proc = std::make_unique<QProcess>();
    QStringList args;
    args << "-interaction=nonstopmode" << "-output-format=dvi" << QString("-output-directory") << filePath
         << QString(fileName).append(".tex");
    qDebug() << "Latex Command: " << latex << "\nArgs: " << args;
    proc.get()->setProgram(latex);
    proc.get()->setArguments(args);
    proc.get()->start();
    proc.get()->waitForStarted(time);

    if(!proc.get()->waitForFinished(time)) {
        proc.get()->kill();
        proc.get()->waitForFinished(-1);
        qDebug() << "Latex.exe timeout (" << time << " ms) -> Kill";
        return 1;
    }
    else {
        if(proc.get()->exitStatus()==QProcess::NormalExit) {
            qDebug() << "Latex finished with normal exit";
        }
        else {
            qDebug() << "Latex finished with error";
        }
    }


    // 2. Step: Convert .dvi to .png
    QString dvipng = QString(pathToTex).append("/dvipng.exe");
    args.clear();
    args <<"-o" << QString(filePath).append(fileName).append(".png")
         << QString(filePath).append(fileName).append(".dvi");
    qDebug() << "Dvipng Command: " << dvipng << "\nArgs: " << args;
    proc.get()->setProgram(dvipng);
    proc.get()->setArguments(args);
    proc.get()->start();
    if(!proc.get()->waitForFinished(time)) {
        qDebug() << "dvipng timeout -> Kill";
        proc.get()->kill();
        proc.get()->waitForFinished(-1);
        return 2;
    }
    else {
        if(proc.get()->exitStatus()==QProcess::NormalExit) {
            qDebug() << "Dvipng finished with normal exit";
        }
        else {
            qDebug() << "Dvipng finished with error";
        }
    }

    reloadPictures();
    return 0;

}

void Latex::createPictures(const QString questionTxt, const QString answerTxt, const QString noteTxt) {
    createPictures(questionTxt, answerTxt);

    createTexFile(noteTxt,tr("note"));
    texToPng(tr("note"));

}

void Latex::createPictures(const QString questionTxt, const QString answerTxt) {
    createTexFile(questionTxt, tr("question"));
    createTexFile(answerTxt, tr("answer"));

    texToPng(tr("question"));
    texToPng(tr("answer"));

}

void Latex::reloadPictures() {
    notePix.load(getFilePath().append(tr("note.png")));
    questionPix.load(getFilePath().append(tr("question.png")));
    answerPix.load(getFilePath().append(tr("answer.png")));
}

void Latex::setLatPath(const QString str) { this->pathToTex = str; }

void Latex::setTime(int newTime) { time = newTime; }
