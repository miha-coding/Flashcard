#ifndef LATEX_H
#define LATEX_H
#include <QPixmap>



/************************************************************************
 * This class provides methods for creation of tex files and its        *
 * convertion to png files.                                             *
 ************************************************************************/

class Latex
{
private:
    QPixmap notePix, questionPix, answerPix, whitePix;
    QString pathToTex,filePath;
    QString firstPart,lastPart; //Text from MainWindow is included between firstPart und lastPart.
    QString firstPartQuestion, lastPartQuestion;  // Analogue to first/lastPart
    int time; // time until timeout of QProcess
public:
    Latex();
    Latex(const QString texDirectory);
    bool createTexFile(const QString content, const QString fileName);
    int texToPng(const QString fileName);
    bool initStrings();
    void createPictures(const QString questionTxt, const QString answerTxt, const QString noteTxt);
    void createPictures(const QString questionTxt, const QString answerTxt);
    void reloadPictures();

    const QPixmap& getNotePix() { return notePix; }
    const QPixmap& getQuestionPix() { return questionPix; }
    const QPixmap& getAnswerPix() { return answerPix; }
    const QPixmap& getWhitePix() { return whitePix; }
    QString getFilePath() const { return filePath; }
    int getTime() const { return time; }

    void setLatPath(const QString str);
    void setTime(int newTime);


};

#endif // LATEX_H
