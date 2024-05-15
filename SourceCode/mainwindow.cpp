#include "mainwindow.h"
#include "addeditquestion.h"
#include "keyword.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>
#include <QIcon>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTime>
#include <cmath>
#include <QFileDialog>
#define VERSION 1.0
#define VERSIONDATE "11.05.2024"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    anzBoxes = 5;
    questions = std::make_shared<Question>();
    kw = std::make_shared<Keyword>(questions);
    addEdit = std::make_unique<AddEditQuestion>(questions, kw);

    hintTxt = solutionTxt = tr("");
    addEdit.get()->resetKwLists(dbKeywords, currentKeywords, selectedKeywords);

    path = new QLabel(tr("Keywords: "), this);
    settingsLab = new QLabel(tr("Question-Nr.: -, Box-Nr.: -, Answered correctly: -"
                                ", Answered incorrectly: -, Left: -"), this);
    question = new QTextEdit(tr("Question:"));
    notices = new QTextEdit(tr("Notices"));
    answer = new QTextEdit(tr("Solution"));
    hintBut = new QPushButton(QIcon(tr("./ico/hint.png")),tr("Get a Hint"), this);
    solutionBut = new QCheckBox(tr("Show Solution"), this);
    correct = new QPushButton(QIcon(tr("./ico/correct.png")),tr("Correct"), this);
    solala = new QPushButton(QIcon(tr("./ico/dontknow.png")),tr("Neither nor"), this);
    wrong = new QPushButton(QIcon(tr("./ico/wrong.png")),tr("Wrong"), this);
    layGroupBox = new QGroupBox(this);
    mainLay = new QGridLayout(layGroupBox);
    txtTex = new QCheckBox(tr("Latex"), this);
    txtTex->setChecked(true);

    questionLab = new QLabel(this);
    noteLab = new QLabel(this);
    answerLab = new QLabel(this);
    questionScroll = new QScrollArea(this);
    answerScroll = new QScrollArea(this);

    // Using new since Constructor in Properties is private and Mainwindow is friend.
    prop = std::unique_ptr<Properties>(new Properties());
    latex = std::make_unique<Latex>(prop.get()->getPathToTex());

    correct->setEnabled(false);
    solala->setEnabled(false);
    wrong->setEnabled(false);

    questionLab->setPixmap(latex.get()->getWhitePix());
    noteLab->setPixmap(latex.get()->getNotePix());
    answerLab->setPixmap(latex.get()->getWhitePix());

    questionScroll->setWidget(questionLab);
    answerScroll->setWidget(answerLab);

    questionScroll->setWidgetResizable(true);
    answerScroll->setWidgetResizable(true);

    //connections
    connect(prop.get(), SIGNAL(accepted()), this, SLOT(apply_settings()));

    connect(hintBut,SIGNAL(clicked()),this,SLOT(getAHint()));
    connect(solutionBut,SIGNAL(toggled(bool)),this,SLOT(getSolutionTxt(bool)));

    connect(correct,SIGNAL(clicked()),this,SLOT(boxUp()));
    connect(wrong,SIGNAL(clicked()),this,SLOT(boxDown()));
    connect(solala,SIGNAL(clicked()),this,SLOT(loadQuestion()));

    //Widget-Settings
    question->setReadOnly(true);
    question->setFixedHeight(68);
    notices->setFixedSize((int) prop.get()->getWidth()*(0.48),(int) prop.get()->getHeight()*0.55);//230
    answer->setFixedSize((int) prop.get()->getWidth()*(0.48),(int) prop.get()->getHeight()*0.55);

    answerScroll->setFixedSize((int) prop.get()->getWidth()*(0.48),(int) prop.get()->getHeight()*0.55);

    answer->setReadOnly(true);

    //Layout: 7 coloums, x rows
    mainLay->addWidget(path,0,0,1,7);
    mainLay->addWidget(settingsLab,1,0,1,7);
    mainLay->addWidget(questionScroll,2,0,1,7);
    mainLay->addWidget(notices,3,0,5,3);
    mainLay->addWidget(answerScroll,3,4,5,3);
    mainLay->addWidget(hintBut,8,0,1,1);
    mainLay->addWidget(solutionBut,8,2,1,1);
    mainLay->addWidget(txtTex,8,1,1,1);
    mainLay->addWidget(correct,8,6,1,1);
    mainLay->addWidget(solala,8,5,1,1);
    mainLay->addWidget(wrong,8,4,1,1);

    layGroupBox->setLayout(mainLay);

    connect(txtTex,SIGNAL(clicked(bool)),this,SLOT(TexOrTxt(bool)));
    this->createMenu();
    this->setFixedSize(prop.get()->getWidth(),prop.get()->getHeight());


    this->setCentralWidget(layGroupBox);
    this->setWindowTitle(tr("Flashcard V%1").arg(VERSION));
    this->setWindowIcon(QIcon(tr("./ico/Flashcard.png")));

}

void MainWindow::about() {
    //Some information about this program
    QString txt = tr("Flashcard Version: %1\n \n Build-date: %2\n\n\
                     Programming language: Qt 6.4.2, created with QtCreator\n\
                     Database driver: SQLite\
                     \n\n Thank you for using Flashcard!").arg(VERSION).arg(VERSIONDATE);
    QMessageBox::information(0,tr("About Flashcard"),txt,QMessageBox::Ok);
}

void MainWindow::addEditQuestion() {
    addEdit.get()->open();
}

void MainWindow::boxDown() {
    //The question could not be solved so the question becomes a lower box number.
    if(questions.get()->getBoxNr()>1)
        questions.get()->boxDown();
    loadQuestion();

}

void MainWindow::boxUp() {
    //The question could be solved, so the question becomes a higher box number.
    if(questions.get()->getBoxNr()<anzBoxes)
        questions.get()->boxUp();
    loadQuestion();
}

void MainWindow::createMenu() {
    //Creates and connects the programms menu-bar.

    mainMenu = this->menuBar()->addMenu(tr("Main"));
    mainMenu->addAction(tr("About"), this, SLOT(about()));
    mainMenu->addAction(QIcon(tr("./ico/preferences.ico")), tr("Settings"),
                        prop.get(), SLOT(open()));
    mainMenu->addAction(QIcon(tr("./ico/quit.ico")),tr("Quit"),this,SLOT(close()));

    //Submenu for keyword-selection
    selKw = new QMenu("Keyword Selection", this);
    selKw->addAction(tr("Select Keywords"), this, SLOT(selectKeywords_menu()));
    selKw->addSeparator();
    selKw->addAction(tr("Reset Selection"), this, SLOT(resetSelection()));

    questionaire = this->menuBar()->addMenu(tr("Questionaire"));
    questionaire->addAction(QIcon(tr("./ico/learning.png")),tr("Start Learning"),
                            this,SLOT(startLearning()));
    questionaire->addMenu(selKw);
    //questionaire->addAction(QIcon(tr("./ico/search.png")),tr("Search Question"),
    //                        this,SLOT(searchQuestion()))->setVisible(false);
    questionaire->addSeparator();
    questionaire->addAction(QIcon(tr("./ico/add.png")),tr("Add/Edit Question"),this,SLOT(addEditQuestion()));
    questionaire->addAction(QIcon(tr("./ico/delete.png")),tr("Delete Question"))->setEnabled(false);

}

void MainWindow::fillMainWidget() {
    // Shows the loaded question on the main widget to learn it.

    //Felder füllen
    QString pathTxt = tr("Keywords: ");
    for(int i=0;i<selectedKeywords.count();i++)
        pathTxt.append(tr(" %1, ").arg(selectedKeywords.at(i)));

    this->path->setText(pathTxt);
    this->question->setText(questions.get()->getQuestion());
    hintTxt = questions.get()->getHint();
    this->solutionTxt = questions.get()->getSolution();
    QString settingsTxt = tr("Question-Nr.: %1, Box-Nr.: %2, Answered correctly: "
                             "%3, Answered incorrectly: %4, Left in Box-Nr %2: %5")\
            .arg(questions.get()->getQuestionId()).arg(questions.get()->getBoxNr())\
            .arg(questions.get()->getAnzRichtig()).arg(questions.get()->getAnzFalsch())\
            .arg(questions.get()->getLeft()+1);
    this->settingsLab->setText(settingsTxt);

    notices->setFocus();
    this->notices->selectAll();

    latex.get()->createPictures(questions.get()->getQuestion(),questions.get()->getSolution());
    latex.get()->reloadPictures();
    this->TexOrTxt(txtTex->isChecked());

    txtTex->setEnabled(true);

    correct->setEnabled(true);
    wrong->setEnabled(true);
    solala->setEnabled(true);

}

void MainWindow::getAHint() {
    QMessageBox::information(0,tr("Hint"),questions.get()->getHint(),QMessageBox::Ok);
}

void MainWindow::getSolutionTxt(bool b) {
    // Shows or hide the solution in main window.

    if(b) {
        if (txtTex->isChecked()) {
            answerLab->setPixmap(latex.get()->getAnswerPix());
            answerScroll->setWidget(answerLab);
            this->answer->setText(questions.get()->getSolution());
        }
        else {
            answerLab->setText(questions.get()->getSolution());
        }
    }
    else {
        if (txtTex->isChecked()) {
            answerLab->setPixmap(latex.get()->getWhitePix());
            answerScroll->setWidget(answerLab);
            this->answer->setText(tr("Solution"));
        }
        else {
            answerLab->setText(tr("Solution"));
        }
    }
}

void MainWindow::loadQuestion() {
    if (questionOnce) {
        //If the list with questions is not initialized yet, initialize it
        if (!questions.get()->getListCreated()) {
            questions.get()->setCreateList(true);
            questions.get()->loadQuestion(selectedKeywords, boxNr, -1);
        }
        //If there is no more question, finish the learning option (load next question)
        if(questions.get()->nextQuestionFromList()==1) {
            QMessageBox::information(0,tr("Learning"),
                                     tr("No more questions available!"),
                                     QMessageBox::Ok);
            correct->setEnabled(false);
            solala->setEnabled(false);
            wrong->setEnabled(false);
            return;
        }
    }
    else {
        bool result = 0;

        do{ // As long as there are questions available (but maybe not loaded in first place)
            result = questions->loadQuestion(selectedKeywords, boxNr, anzBoxes);
        } while(questions.get()->getLeft()+1!=0 && result==false); // result == 0 -> no question
                                                                   // in this category;

        if (questions.get()->getLeft()+1==0) {
            QMessageBox::warning(0,tr("Start Learning"),
                                 tr("No Question found!"),QMessageBox::Ok);
            correct->setEnabled(false);
            solala->setEnabled(false);
            wrong->setEnabled(false);
            return;
        }
    }

    this->solutionBut->setChecked(false);
    this->notices->setText(tr("Notices"));
    this->fillMainWidget();

}

void MainWindow::apply_settings() {
    prop.get()->loadProperties();
    latex.get()->setLatPath(prop.get()->getPathToTex());
    latex.get()->setTime(prop.get()->getTimeOut());
    qDebug() << "MainWindow::apply_settings: New settings applied.";

}

void MainWindow::resetSelection() {
    // No selected Kws -> Show all Keywords
    currentKeywords = questions.get()->getKeywordsFromDb();
    questions.get()->saveCurrentKw(QStringList(tr("")));
}


void MainWindow::selectKeywords_menu() {
    kw.get()->selectKeywords(dbKeywords);
}


void MainWindow::startLearning() {
    auto dialog = std::make_unique<QDialog>();
    QComboBox *boxNumber = new QComboBox(dialog.get());
    QLabel *boxNumTxt = new QLabel(tr("0 means Question from all Boxes, else only from BoxNr.:"));
    QLabel *keywordTxt = new QLabel(tr("Only Question with following\n Keywords:"));
    QPushButton *start, *cancel;
    QGridLayout *gridLay = new QGridLayout(dialog.get());
    QDialogButtonBox *dialButBox = new QDialogButtonBox(dialog.get());
    keywordList = new QListWidget(dialog.get());
    keywordList->setFixedSize(140,100);
    selectedList = new QListWidget(dialog.get());
    selectedList->setFixedSize(140,100);
    QCheckBox *questionOneTime = new QCheckBox(tr("Get every Question only once"), dialog.get());

    for(int i = 0;i<=this->anzBoxes;i++) boxNumber->addItem(QVariant(i).toString());

    addEdit.get()->resetKwLists(dbKeywords, currentKeywords, selectedKeywords);
    keywordList->addItems(currentKeywords);

    start = new QPushButton(tr("Start"), dialog.get());
    cancel = new QPushButton(tr("Cancel"), dialog.get());

    dialButBox->addButton(start, QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancel, QDialogButtonBox::RejectRole);

    gridLay->addWidget(keywordTxt,         0,2,1,2);
    gridLay->addWidget(keywordList,        1,0,1,2);
    gridLay->addWidget(selectedList,       1,2,1,2);
    gridLay->addWidget(boxNumTxt,          2,0,1,4);
    gridLay->addWidget(boxNumber,          3,0,1,4);
    gridLay->addWidget(questionOneTime,    4,0,1,4);
    gridLay->addWidget(dialButBox,         5,0,1,4, Qt::AlignRight);

    dialog.get()->setLayout(gridLay);

    connect(keywordList,SIGNAL(clicked(QModelIndex)),this,SLOT(dbToSel(QModelIndex)));
    connect(selectedList,SIGNAL(clicked(QModelIndex)),this,SLOT(selToDB(QModelIndex)));
    connect(start,SIGNAL(clicked()),dialog.get(),SLOT(accept()));
    connect(cancel,SIGNAL(clicked()),dialog.get(),SLOT(reject()));

    int ret = dialog.get()->exec();
    if(ret==QDialog::Rejected) return;
    if(selectedList->count()==0) return;

    questionOnce = questionOneTime->isChecked() ? true : false;
    //selectedList->item(0)->text();
    this->boxNr = boxNumber->currentText().toInt();
    questions.get()->setListCreated(false);
    this->loadQuestion();

}



void MainWindow::dbToSel(const QModelIndex& mod) {
    //Swaps a keyword from dbKeyword list to selection list in method addEditQuestion
    this->selectedKeywords.append(mod.data().toString());
    this->selectedKeywords.sort(Qt::CaseInsensitive);
    this->currentKeywords.removeAll(mod.data().toString());

    //Listen aktualisieren
    this->keywordList->clear();
    this->keywordList->addItems(currentKeywords);
    this->selectedList->clear();
    this->selectedList->addItems(selectedKeywords);
}

void MainWindow::selToDB(const QModelIndex& mod) {
    //Swaps a keyword from selection list to dbKeyword list in method addEditQuestion
    currentKeywords.append(mod.data().toString());
    currentKeywords.sort(Qt::CaseInsensitive);
    selectedKeywords.removeAll(mod.data().toString());

    //Listen aktualisieren
    this->keywordList->clear();
    this->keywordList->addItems(currentKeywords);
    this->selectedList->clear();
    this->selectedList->addItems(selectedKeywords);
}

void MainWindow::TexOrTxt(bool b) {
    // This function switches the question and answer box from Latex (picture) to
    // text and vise versa depending if the Latex-Button is checked.
    if (!b) { // Show text
        questionLab->setText(questions.get()->getQuestion());
        if (solutionBut->isChecked())
            answerLab->setText(questions.get()->getSolution());
        else
            answerLab->setText("Solution");
    }
    else { // Use Latex
        questionLab->setPixmap(latex.get()->getQuestionPix());
        if (solutionBut->isChecked())
            answerLab->setPixmap(latex.get()->getAnswerPix());
        else
            answerLab->setPixmap(latex.get()->getWhitePix());
    }
    questionScroll->setWidget(questionLab);
    answerScroll->setWidget(answerLab);
}

/*
// Testen der Zufallsverteilung
int i[5];
for( int j = 0;j<5;j++) i[j]=0;
for(int k=0;k<1000000;k++) {

    switch (questions.getRandomBoxNumber()) {
        case 1:
            i[0]++;
            break;
        case 2:
            i[1]++;
            break;
        case 3:
            i[2]++;
            break;
        case 4:
            i[3]++;
            break;
        case 5:
            i[4]++;
            break;

    }

}
for( int j = 0;j<5;j++) qDebug() << j << tr(": ") << i[j];
*/

