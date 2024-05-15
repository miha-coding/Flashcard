#ifndef QUESTION_H
#define QUESTION_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QMap>
#include <QStringList>


/************************************************************************
 * This implements questions and the writing/saving of the questions in *
 * a database.                                                          *
 ************************************************************************/


class Question : public QObject
{
    Q_OBJECT

private:
    QString question, solution, hint;
    QSqlDatabase db;
    int id, anzRichtig, anzFalsch, boxNr, left;
    QMap<QString,QString> kwMap;
    QStringList kwList;
    QList<int> questionIdList;
    bool listCreated,createList;

    // Database methods
    void createDatabase();
    bool openDatabase();

    bool beginTransaction(QSqlQuery& query, QString method="");
    bool commit(QSqlQuery& query, QString method="");
    bool rollback(QSqlQuery& query, QString method="");
    bool queryExec(QSqlQuery& query, QString method="", QString stmt="");
public:
    Question(QObject *parent = nullptr);
    bool writeKeyword(const QString keyWord);
    bool addQuestion(const QStringList& kwList, const QString q,
                     const QString h, const QString s);
    bool deleteKeyword(const QString keyWord);
    bool deleteQuestion(const int id);
    QStringList getKeywordsFromDb();
    QList<int> getPreparedKeywords(const QStringList& selectedKeywords);
    bool loadQuestion(const QStringList& keyWords, const int boxNum, const int maxBoxes);
    int loadQuestionFromDB(const QStringList& keyWords, const int boxNum, const int maxBoxes);
    int getRandomBoxNumber(const int anzBoxes);
    std::unique_ptr<QTableWidget> searchQuestion(const QString dbColoumn, const QString searchTxt);
    bool updateQuestion(const int id, const QString qTxt, const QString hTxt,
                        const QString sTxt, const QStringList& kwList);

    int nextQuestionFromList();

    //get/set Methoden
    QString getSolution() const { return this->solution; }
    QString getHint() const { return this->hint; }
    QString getQuestion() const { return this->question; }
    int getQuestionId() const { return id; }
    int getAnzRichtig() const { return anzRichtig; }
    int getAnzFalsch() const { return anzFalsch; }
    int getBoxNr() const { return boxNr; }
    int getId() const { return id; }
    int getLeft() const {return left; }
    bool getListCreated() const { return listCreated; }
    QStringList getKwList() const { return kwList; }

    void setAnswer(QString s) { this->solution = s; }
    void setHint(QString h) { this->hint = h; }
    void setQuestion(QString q) { this->question = q; }
    void setCreateList(bool b) { this->createList = b; }
    void setListCreated(bool b) { this->listCreated = b; }

    void changeDB();
public slots:
    int boxUp();
    int boxDown();
    bool loadQuestion(int id);
    int idExists(int id);
    bool saveCurrentKw(QStringList curKw);
    QStringList getCurrentKws(bool *noCurrentKws=nullptr);

};

#endif // QUESTION_H
