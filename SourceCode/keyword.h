#ifndef KEYWORD_H
#define KEYWORD_H

#include<question.h>

#include<QObject>
#include<QListWidget>


class Keyword : public QObject {

    Q_OBJECT

private:
    QStringList dbKeywords,
        selectedKeywords;
    QListWidget *keywordList,
        *selectedList;

    std::shared_ptr<Question> questions;

public:
    Keyword(std::shared_ptr<Question> q, QObject *parent=nullptr);
    void minusList(QStringList *all, QStringList *cur);

public slots:
    bool addKeyword();
    const QString deleteKeyword();
    void removeItemFromAllLists(const QString kw);
    void selectKeywords(QStringList& newSelKws);
    void selToDB(const QModelIndex& mod);
    void dbToSel(const QModelIndex& mod);
};

#endif // KEYWORD_H
