#ifndef ADDEDITQUESTION_H
#define ADDEDITQUESTION_H

#include<question.h>
#include<keyword.h>

#include<QDialog>
#include<QLabel>
#include<QTextEdit>
#include<QGridLayout>
#include<QListWidget>
#include<memory>


class AddEditQuestion : public QDialog {

    Q_OBJECT

private:
    std::shared_ptr<Question> questions;
    std::shared_ptr<Keyword> kw;
    QPushButton *saveBut, *quitBut, *searchBut, *addBut,
        *selBut, *delBut, *delQuestBut;
    QLabel *questionLab, *hintLab, *answerLab;
    QTextEdit *quest, *ans, *hint;
    QGridLayout *dialogLay;
    int id;

    QStringList dbKeywords, // CurrentKws from DB
        currentKeywords,    // 1. = dbKeywords, then dbKeywords without selectedKeywords
        selectedKeywords;   // selected from currentKeywords

    QListWidget *keywordList,   // contains currentKeywords
        *selectedList;  // contains selectedKeywords

    //search and Load Question
    bool questionLoaded,
        delQuestion;
    std::unique_ptr<QTableWidget> tab = nullptr;

public:
    AddEditQuestion(std::shared_ptr<Question> q, std::shared_ptr<Keyword> k, QDialog *parent=nullptr);
    void resetKwLists();
    void resetKwLists(QStringList& dbKws, QStringList& curKw, QStringList& selKw);
    void updateAddEditLists();
    void minusList(QStringList *all, QStringList *cur);
    void removeItemFromAllLists(const QString kw);

public slots:
    int searchQuestion();
    void evaluateItem(int r, int c);
    void flipDelQuestion() { delQuestion ? delQuestion = false:delQuestion = true; }
    void setLoadedQuestionTrue() { questionLoaded=true; }
    void dbToSel_addEdit(const QModelIndex& mod);
    void selToDB_addEdit(const QModelIndex& mod);
    void addKeyword();
    void deleteKeyword();
    void selectKeywords();
    void open() override;
    void processResult(int result);
};

#endif // ADDEDITQUESTION_H
