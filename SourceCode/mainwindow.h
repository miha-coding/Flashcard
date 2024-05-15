#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "question.h"
#include "latex.h"
#include "properties.h"
#include "addeditquestion.h"
#include <QMainWindow>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTextEdit>
#include <QSqlDatabase>
#include <QScrollArea>
#include <memory>


class AddEditQuestion;
class Keyword;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    std::shared_ptr<Question> questions;
    QLabel *path,
           *settingsLab,
           *anzLeftQuestions;
    QMenu *mainMenu,
          *questionaire,
          *selKw;
    QTextEdit *question,
              *notices,
              *answer;
    QPushButton *hintBut;
    QCheckBox *solutionBut;
    bool questionOnce;
    QPushButton *correct,
                *solala,
                *wrong;
    QGridLayout *mainLay;
    QGroupBox *layGroupBox;
    QSqlDatabase db;
    int anzBoxes,
        boxNr;
    QString hintTxt,
            solutionTxt;
    std::unique_ptr<Latex> latex;
    QLabel *questionLab,
           *noteLab,
           *answerLab;
    QScrollArea *questionScroll,
                *noteScroll,
                *answerScroll;

    std::shared_ptr<Keyword> kw;
    std::unique_ptr<AddEditQuestion> addEdit;

    QStringList dbKeywords,         // CurrentKws from DB
                currentKeywords,    // 1. = dbKeywords, then dbKeywords without selectedKeywords
                selectedKeywords;   // selected from currentKeywords

    QListWidget *keywordList,   // contains currentKeywords
                *selectedList;  // contains selectedKeywords

    //Properties
    std::unique_ptr<Properties> prop;

    //Switch between plain text and tex
    QCheckBox *txtTex;

    void createMenu();

public:
    MainWindow(QWidget *parent = nullptr);


    //Get/Set
    QStringList getDBKeywords() { return dbKeywords; }
    QStringList getSelectedKeywords () { return selectedKeywords; }

    void fillMainWidget();
    void setDBKeywords(QStringList str) { dbKeywords = str; }
    void setSelectedKeywords(QStringList str) { selectedKeywords = str; }


public slots:
    void about();
    void addEditQuestion();
    void boxUp();
    void boxDown();
    void getAHint();
    void getSolutionTxt(bool b);
    void loadQuestion();
    void apply_settings();
    void resetSelection();
    void selectKeywords_menu();
    void startLearning();

    void selToDB(const QModelIndex& mod);
    void dbToSel(const QModelIndex& mod);

    void TexOrTxt(bool b);

};

#endif // MAINWINDOW_H
