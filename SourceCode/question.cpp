#include "question.h"
#include <QMessageBox>
#include <QSqlError>
#include <QList>
#include <QSqlQuery>
#include <QString>
#include <QTableWidget>
#include <QHeaderView>
#include <QTime>
#include <QVariant>
#include <cmath>
#include <memory>
#include <QRandomGenerator>
#include <QFile>

#include <QDebug>
#include <QSqlResult>


Question::Question(QObject *parent) : QObject(parent)
{
    //Data for Database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName("localhost");
    db.setDatabaseName("Flashcard");
    db.setUserName("FlashcardUser");
    db.setPassword("Fdb8933CKL");

    // Check if DB exists. If not create it.
    qDebug() << "Question::Question: DB existing?" << QFile::exists("./Flashcard");
    if(!QFile::exists("./Flashcard")) this->createDatabase();

    // Searching options for searching a question
    kwMap["number"] = "id";
    kwMap["Question-text"] = "question";
    kwMap["Box-Nr"] = "karteiNr";

    question = solution = hint = tr("");
    id = anzRichtig = anzFalsch = boxNr = left = 0;
    listCreated = false;
    createList = false;
    questionIdList = QList<int>();

}

bool Question::addQuestion(const QStringList &kwList, const QString q,
                           const QString h, const QString s)
{
    QList<int> kwIds = getPreparedKeywords(kwList);
    this->openDatabase();
    QSqlQuery query;

    // Add question to DB Questions and QuestionKeywords that connects
    // Questions to existing Keywords.
    beginTransaction(query, "addQuestion");

    query.prepare(tr("INSERT INTO Questions(anzRichtig,anzFalsch,"
                     "karteiNr,txtRbCb,question,hint,answer) "
                     "VALUES (0, 0, 1 ,0, :q , :h , :s );"));
    query.bindValue(tr(":q"), q);
    query.bindValue(tr(":h"), h);
    query.bindValue(tr(":s"), s);

    bool success = queryExec(query, "addQuestion");

    if(!success) {
        rollback(query, "addQuestion");
        db.close();
        return false;
    }

    // Get autoincremented id (needed for connecting question to keyword)
    int id = query.lastInsertId().toInt();
    QString queryTxt = QString("INSERT INTO QuestionKeywords(questionId, keywordId) "
                               "VALUES (%1, ").arg(id);

    QString kwQueryTxt;
    // One entry for each Keyword
    for(int i=0; i<kwIds.size(); ++i) {
        kwQueryTxt = QString(queryTxt).append(QString("%1").arg(kwIds.at(i))).append(");");
        if(!queryExec(query, "addQuestion", kwQueryTxt)) {
            rollback(query, "addQuestion");
            db.close();
            return false;
        }
    }


    // All fine -> commit
    if(commit(query, "addQuestion")) {
        db.close();
        return true;
    }
    else {
        db.close();
        return false;
    }

}

void Question::createDatabase() {
    // Creates every Database (i.e. Questions, Keywords, QuestionKeywords,
    // currentKeywords)

    bool success;
    if(!openDatabase()) return;

    QSqlQuery query;

    success = query.exec(tr("CREATE TABLE Questions (                       "
               "id            INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,   "
               "anzRichtig    INTEGER NOT NULL DEFAULT 0,                   "
               "anzFalsch     INTEGER NOT NULL DEFAULT 0,                   "
               "karteiNr      INTEGER NOT NULL DEFAULT 1,                   "
               "question      TEXT NOT NULL,                                "
               "hint          TEXT NOT NULL DEFAULT '-',                    "
               "answer        TEXT NOT NULL,                                "
               "pngLink       TEXT                                          "
           ");"));
    if(!success) {
        QMessageBox::critical(0,tr("Database Error"),
                              tr("Error! Database could not create Table Questions!"),
                              QMessageBox::Ok);
        qDebug() << query.lastError().text();
        db.close();
        return;
    }
    qDebug() << "Question erzeugt";

    success = query.exec(tr("CREATE TABLE `Keywords` (                  "
               "id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,   "
               "name      VARCHAR(50) NOT NULL                          "
           ");"));
    if(!success) {
        QMessageBox::critical(0,tr("Database Error"),
                              tr("Error! Database could create Table Keywords!"),
                              QMessageBox::Ok);
        qDebug() << query.lastError().text();
        db.close();
        return;
    }
    qDebug() << "Keywords erzeugt";

    success = query.exec(tr("CREATE TABLE QuestionKeywords (                  "\
                            "questionId       INTEGER NOT NULL,               "\
                            "keywordId        INTEGER NOT NULL,               "\
                            "FOREIGN KEY(questionId) REFERENCES Questions(id),"\
                            "FOREIGN KEY(keywordId) REFERENCES Keywords(id)   "\
                            ");"));
    if(!success) {
        QMessageBox::critical(0,tr("Database Error"),
                              tr("Error! Database could create Table QuestionKeywords!"),
                              QMessageBox::Ok);
        qDebug() << query.lastError().text();
        db.close();
        return;
    }
    qDebug() << "QuestionKeywords erzeugt";

    success = query.exec(tr("CREATE TABLE CurrentKws ("
                            "name VARCHAR(50) NOT NULL PRIMARY KEY"\
                            ");"));
    if(!success) {
        QMessageBox::critical(0,tr("Database Error"),
                              tr("Error! Database could create Table CurrentKws!"),
                              QMessageBox::Ok);
        qDebug() << query.lastError().text();
        db.close();
        return;
    }
    qDebug() << "currentKeywords erzeugt";



    qDebug() << "DB erzeugt";
    QMessageBox::information(0,tr("Database"),
                             tr("Database successfully created!"), QMessageBox::Ok);

    db.close();
}

bool Question::deleteKeyword(const QString keyWord) {
    openDatabase();
    QSqlQuery query;

    // Get the keyword-id
    query.prepare("SELECT id FROM Keywords WHERE name = :name ");
    query.bindValue(":name", keyWord);
    // Delete also all entries in QuestKeywords with this keyword

    if(!queryExec(query, "deleteKeyword")) {
        db.close();
        return false;
    }

    query.next();
    int id = query.value(0).toInt();

    // Delete Keyword with the obtained id and all entries from
    // QuestionKeywords with this keyword
    if(!beginTransaction(query, "deleteKeyword")) {
        db.close();
        return false;
    }
    bool success = queryExec(query, "deleteKeyword",
                         QString("DELETE FROM Keywords WHERE id=%1").arg(id));
    success = success && queryExec(query, "deleteKeyword",
          QString("DELETE FROM QuestionKeywords WHERE keywordId=%1").arg(id));
    query.clear();
    query.prepare("DELETE FROM currentKws WHERE name= :name ;");
    query.bindValue(":name", keyWord);
    success = success && queryExec(query, "deleteKeyword");

    if(!success) {
        qDebug() << "Question::deleteKeyword: ROLLBACK!";
        rollback(query, "deleteKeyword");
        db.close();
        return false;
    }
    else {
        if(commit(query, "deleteKeyword")) {
            db.close();
            return true;
        }
        db.close();
        return false;
    }
}

bool Question::deleteQuestion(const int id) {
    // return: 1->success, 0->DB Error, -1 ->
    openDatabase();
    QSqlQuery query;

    if(!beginTransaction(query, "deleteQuestion")) {
        db.close();
        return false;
    }

    query.prepare("DELETE FROM Questions WHERE id = :id ;");
    query.bindValue(":id", id);

    bool success = queryExec(query, "deleteQuestion");

    query.prepare("DELETE FROM QuestionKeywords WHERE QuestionId = :id ;");
    query.bindValue(":id", id);
    success = success && queryExec(query, "deleteQuestion");


    if(success) {
        if(commit(query, "deleteQuestion")) {
            QMessageBox::information(0,tr("Delete Question"),
                                     tr("Question successfully deleted!"),
                                     QMessageBox::Ok);
            db.close();
            return true;
        }
        db.close();
        return false;
    }

    qDebug() << "Question::deleteQuestion: ROLLBACK!";
    rollback(query, "deleteQuestion");
    db.close();

    return false;
}

QStringList Question::getKeywordsFromDb() {
    // Returns a sorted list of all Keywords

    QStringList strList;
    if(!this->openDatabase()) return strList;
    QSqlQuery query;
    if(!queryExec(query, "getKeywordsFromDb", "SELECT name FROM Keywords")) {
        db.close();
        return strList;
    }

    while(query.next()) {
        strList.append(query.value(tr("name")).toString());
    }
    db.close();
    strList.sort(Qt::CaseSensitive);

    return strList;
}

QList<int> Question::getPreparedKeywords(const QStringList& selectedKeywords) {
    // Get and return the ids for all keywords in the list selectedKeywords.

    QList<int> kwIdList;
    if(!this->openDatabase()) return kwIdList;
    QSqlQuery query;
    QString queryTxt = "SELECT id FROM Keywords WHERE name = '";

    for(int i=0;i<selectedKeywords.count()-1;i++) {
        queryTxt.append(selectedKeywords.at(i));
        queryTxt.append(tr("' OR name= '"));
    }

    queryTxt.append(selectedKeywords.at(selectedKeywords.count()-1));
    queryTxt.append(tr("'; "));
    qDebug() << queryTxt;
    if(!queryExec(query, "getPreparedKeywords", queryTxt)) {
        qDebug() << tr("Query coud not be executed!");
        db.close();
        return kwIdList;
    }


    while(query.next()) {
        kwIdList.append(query.value("id").toInt());
    }
    query.clear();
    db.close();

    return kwIdList;
}

int Question::getRandomBoxNumber(const int anzBoxes) {
    /* Let n := anzBoxes.
     * Aim ask question from box i i-times as often as a question from box n:
     * Put Box n one Time in a container, Box n-1 two times, etc.
     * There are a total of m := 0.5*n*(n+1) in the container.
     * So questions that are in higher boxes (well answered questions) are asked
     * more seldom than questions that are not well answered (lower boxes).
     * Draw a box and ask a question from this box number.
     * Bijective function: 1->Box n, 2->Box n-1, 3->Box n-1, 4->Box n-2, ...
     * Draw equally likely a number between 1 and m = 0.5*n*(n+1)
     * Get the box via: m = 0.5*n*(n+1) <=> n^2+n-2m = 0 (use pq-formula)
     * -> n = ceil(0.5*(sqrt(1+8m)-1))
    */

    int m = anzBoxes*(anzBoxes+1)*0.5;
    int randbox = QRandomGenerator::global()->bounded(1, m+1); // [1,m+1)=[1,m]
    int n = ceil(0.5*(sqrt(1+8*randbox)-1));

    return (anzBoxes-(n-1)); //To get correct number between 1 and 5.
}

int Question::idExists(int id) {
    // Checks if a question with id id exists.
    // Return: 1 -> exists; 0 -> does not exist; -1 -> Error

    openDatabase();
    QSqlQuery query;
    query.prepare(tr("SELECT id FROM Questions WHERE id = :id "));
    query.bindValue(tr(":id"),id);

    if(!queryExec(query, "idExists")) {
        db.close();
        return -1;
    }

    if(query.next()) {
        db.close();
        return 1; //Frage existiert
    }
    db.close();
    return 0;
}

int Question::loadQuestionFromDB(const QStringList &keyWords, const int boxNum, const int maxBoxes) {

    QList<int> keyWordIds = getPreparedKeywords(keyWords);
    if(keyWordIds.isEmpty()) {
        qDebug() << "Question::loadQuestionFromDB: keyWordIds is empty!";
        return -1;
    }

    qDebug() << tr("Question::loadQuestionsFromDB: KeywordIds\n") << keyWordIds;

    QList<int> boxNrList;
    for (int i=1;i<=maxBoxes;i++) {
        boxNrList.append(i);
    }
    int randnum = -1;

    QSqlQuery query;
    int querySize;
    QString queryStr;
    QString colNames = "id, anzRichtig, anzFalsch, karteiNr, question, hint, answer";
    do {
        query.clear();
        // Set statement dependend on what options were chosen in startLearning.
        if (maxBoxes==-1) { // Every Question only once.
            if (boxNum==0) { // Questions from all boxes.
                queryStr = QString("SELECT %1 FROM Questions as Q JOIN "
                              "QuestionKeywords as QK ON Q.id = QK.questionId "
                              "WHERE keywordId in (").arg(colNames);
            }
            else { // Questions from one specific box
                queryStr = QString("SELECT %1 FROM Questions as Q JOIN "
                              "QuestionKeywords as QK ON Q.id = QK.questionId "
                              "WHERE karteiNr = %2 and keywordId in (")
                               .arg(colNames).arg(boxNum);
            }
        }
        else { // Questions can be asked multiple times.

                if(boxNrList.count()==0) { // No boxes left in boxNrList
                    db.close();
                    return 0;
                }

                if(boxNum == 0) { // Draw a random box number (all boxes allowed).
                    int rndnm = getRandomBoxNumber(boxNrList.count());
                    randnum = boxNrList.at(rndnm-1);
                    // If the program returns to this line, we know that there
                    // is no question in this box. Remove it so that it can't
                    // be drawn the next time.
                    boxNrList.removeAt(rndnm-1);
                    queryStr = tr("SELECT %1 FROM Questions Q JOIN "
                                  "QuestionKeywords as QK ON Q.id = QK.questionId "
                                  "WHERE karteiNr = %2 and keywordId in (")
                                   .arg(colNames).arg(randnum);
                }
                else {
                    queryStr = tr("SELECT %1 FROM Questions Q JOIN "
                                  "QuestionKeywords as QK ON Q.id = QK.questionId "
                                  "WHERE karteiNr = %2 and keywordId in (")
                                   .arg(colNames).arg(boxNum);
                }

        }

        // Add keywordIds to the SQL statement
        queryStr.append(QString("%1").arg(keyWordIds.at(0)));
        for(int i=1;i<keyWordIds.count();i++) {
            queryStr.append(QString(", %1").arg(keyWordIds.at(i)));
        }
        queryStr.append(") GROUP BY id;"); // Group by id to prevent that questions
        // with multiple keywords appear multiple times in the results.
        qDebug() << queryStr;
        // SQL statement completed.

        if(!openDatabase()) return -1;
        if(!queryExec(query, "loadQuestionFromDB", queryStr)) {
            db.close();
            return -1;
        }

        if(createList) {
            // If every question should be asked only once, all the questions
            // are written in a list and later, delete all the questions that
            // were asked.
            questionIdList.clear();
            while(query.next()) {
                questionIdList.append(query.value("id").toInt());
            }
            qDebug() << "Created questionIdList-size: " << questionIdList.size();
            listCreated=true;
            createList=false;
        }

        // Determine number of matching questions.
        // (query.size() does not work (it might not be supported by SQLite))
        querySize = 0;
        QSqlQuery countQuery;
        countQuery.exec(queryStr);
        while(countQuery.next())
            querySize++;

        left = querySize-1; //Damit es bei der anzeige passt
        qDebug () << querySize;

        /* If no questions have been found so far (left<=0) but there are still
         * boxes left that have not yet been looked up (BoxNrList not empty),
         * run through the do block again, but only in the case that arbitrary
         * boxes have been selected (boxNum==0) and questions may be asked
         * repeatedly (maxBoxes>-1).
         * If questions may be asked repeatedly, but only from a specific box,
         * a list is created. Therefore, the do block does not have to be run
         * through again in this case.
         */
    } while(boxNum==0 && maxBoxes>-1 && !boxNrList.isEmpty() && left<0);
    if(querySize == 0) {
        qDebug() << tr("No questions found.");
        db.close();
        return 0;
    }

    // Choose a question from the results randomly.
    int randNum = 0;
    if((querySize-1)!=0)
        randNum = QRandomGenerator::global()->bounded(querySize);
    for(int i=0;i<=randNum;i++)
        query.next();

    // Set attributes.
    this->setQuestion(query.value("question").toString());
    hint = query.value("hint").toString();
    solution = query.value("answer").toString();

    id = query.value("id").toInt();
    anzRichtig = query.value("anzRichtig").toInt();
    anzFalsch = query.value("anzFalsch").toInt();
    boxNr = query.value("karteiNr").toInt();

    db.close();
    return 1;
}

bool Question::loadQuestion(const QStringList &keyWords, const int boxNum, const int maxBoxes) {
    int retVal = loadQuestionFromDB(keyWords, boxNum, maxBoxes);

    if(questionIdList.isEmpty() && retVal<1)
        return false;
    else
        return true;
}

bool Question::loadQuestion(int id) {
    openDatabase();
    QSqlQuery query;
    if(!queryExec(query, "loadQuestion",
                   QString("SELECT id, anzRichtig, anzFalsch, karteiNr, question, "
                      "hint, answer FROM Questions WHERE id = %1").arg(id)))
    {
        db.close();
        return false;
    }
    if(!query.first()) return false;

    // Set attributes
    this->setQuestion(query.value("question").toString());
    hint = query.value("hint").toString();
    solution = query.value("answer").toString();

    this->id = query.value("id").toInt();
    anzRichtig = query.value("anzRichtig").toInt();
    anzFalsch = query.value("anzFalsch").toInt();
    boxNr = query.value("karteiNr").toInt();

    // Now the keywords...
    QString queryStr = QString("SELECT name FROM Keywords as K JOIN "
                               "QuestionKeywords as QK ON K.id = QK.keywordId "
                               "WHERE QK.QuestionId = %1").arg(id);
    if(!queryExec(query, "loadQuestion", queryStr)) {
        db.close();
        return false;
    }
    kwList.clear();
    while(query.next()) {
        kwList.append(query.value("name").toString());
    }

    db.close();
    return true;
}

int Question::nextQuestionFromList() {
    if(!listCreated) return -1;
    qDebug() << "Question::nextQuestionFromList: Sizeof QuestionIDList"
             << questionIdList.size();

    if(questionIdList.size()==0) return 1;

    //search and load a question randomly from the list with the left questions.
    int randNum = QRandomGenerator::global()->bounded(questionIdList.size());
    this->loadQuestion(questionIdList.at(randNum));
    questionIdList.removeAt(randNum);
    left=questionIdList.size();
    return 0;
}

bool Question::openDatabase() {
    if(!db.open()) {
        QMessageBox::critical(0,tr("Database Error"),
                              tr("Error! Database could not be opened:\n %1")
                                  .arg(db.lastError().text()),QMessageBox::Ok);
        return false;
    }
    qDebug() << tr("DB erfolgreich geöffnet");
    return true;
}

bool Question::writeKeyword(const QString keyWord) {
    openDatabase();

    QSqlQuery query;
    query.prepare(tr("INSERT INTO Keywords(name) VALUES (:name );"));
    query.bindValue(tr(":name"), keyWord);

    bool success = queryExec(query, "writeKeyword");

    query.clear();
    query.prepare("INSERT INTO currentKws(name) VALUES (:name);");
    query.bindValue(":name", keyWord);
    queryExec(query, "write Keyword");  // Without checking for success because
                                        // Kw can be added later to the list.


    db.close();
    if(success) return true;
    return false;

}

//Slots
int Question::boxUp() {
    openDatabase();
    QSqlQuery query;
    query.prepare(tr("UPDATE Questions SET karteiNr = %1 WHERE id = %2")\
                  .arg(this->boxNr+1).arg(this->id));
    if(!queryExec(query, "boxUp")) {
        db.close();
        return -1;
    }
    db.close();
    return 1;
}

int Question::boxDown() {
    if(this->boxNr==0) return 0;
    openDatabase();
    QSqlQuery query;
    query.prepare(tr("UPDATE Questions SET karteiNr = %1 WHERE id = %2")\
                  .arg(this->boxNr-1).arg(this->id));
    if(!queryExec(query, "boxDown")) {
        db.close();
        return -1;
    }
    db.close();
    return 1;
}

QStringList Question::getCurrentKws(bool *noCurrentKws) {
    openDatabase();
    QSqlQuery query;
    queryExec(query, "getCurrentKws", "SELECT name FROM currentKws;");

    QStringList list;
    while(query.next()) {
        list.append(query.value("name").toString());
    }

    if(!list.isEmpty()) { // There are currentKws
        db.close();
        if(noCurrentKws!=nullptr) *noCurrentKws=false;
        return list;
    }

    // There are no current Kws => All Kws are currentKws
    queryExec(query, "getCurrentKws", "SELECT name FROM Keywords;");
    while(query.next()) {
        list.append(query.value("name").toString());
    }
    db.close();
    if(noCurrentKws!=nullptr) *noCurrentKws=true;
    return list;
}

bool Question::saveCurrentKw(QStringList curKw) {

    openDatabase();
    QSqlQuery query;

    // If there are no new Kws -> delete every keyword from CurrentKws.
    if(curKw.isEmpty()) {
        bool s = (queryExec(query, "saveCurrentKw", "DELETE FROM CurrentKws;"));
        db.close();
        return (s ? true : false);
    }

    if(!beginTransaction(query, "saveCurrentKw")) {
        db.close();
        return false;
    }
    queryExec(query, "saveCurrentKw", "DELETE FROM CurrentKws;");

    QString queryStr = "INSERT INTO CurrentKws(name) VALUES ( ";
    for(int i=0;i<curKw.size()-1;++i) {
        queryStr.append(QString(":name%1 ), ( ").arg(i));
    }
    queryStr.append(QString(":name%1 );").arg(curKw.size()-1));
    query.prepare(queryStr);

    // Bind values
    for(int i=0;i<curKw.size()-1;++i) {
        query.bindValue(QString(":name%1").arg(i), curKw.at(i));
    }
    query.bindValue(QString(":name%1").arg(curKw.size()-1), curKw.at(curKw.size()-1));

    qDebug() << "Question::saveCurrentKw query: " << queryStr << "\n"
             << query.boundValues().size();

    bool success = queryExec(query, "saveCurrentKw");
    if(!success) {
        rollback(query, "saveCurrentKw");
    }
    else {
        success = commit(query, "saveCurrentKw");
    }
    db.close();
    if(success) { qDebug() << tr("Question::saveCurrentKw: CurrentKws saved"); return true; }
    else { qDebug() << tr("Question::saveCurrentKw: Error!"); return false; }

}

std::unique_ptr<QTableWidget> Question::searchQuestion(const QString dbColoumn, const QString searchTxt) {
    auto table = std::make_unique<QTableWidget>(0, 2);
    openDatabase();
    QSqlQuery query;
    QString mapTxt = kwMap[dbColoumn];
    QString txt;

    if(mapTxt=="question") {  // With LIKE since we search for a text.
        txt = tr("Select id, anzRichtig, anzFalsch, karteiNr, question, hint, "
                 "answer FROM Questions WHERE question LIKE :searchTxt;");
        query.prepare(txt);
        query.bindValue(tr(":searchTxt"),QString(tr("%%1%").arg(searchTxt)));
    }
    else { // Without like because searchTxt is a number
        txt = tr("Select id, anzRichtig, anzFalsch, karteiNr, question, hint, "
                 "answer FROM Questions WHERE %1 = :searchTxt;").arg(mapTxt);
        query.prepare(txt);
        query.bindValue(tr(":searchTxt"), searchTxt);
    }

    bool success = queryExec(query, "searchQuestion");
    if(!success) {
        QMessageBox::critical(0,tr("Search Question"),
                              tr("SQL Error!"),QMessageBox::Ok);
        db.close();
        return table;
    }

    // Insert found questions in result table
    int i = 0;
    while(query.next()) {
        table->insertRow(i);
        table->setItem(i,0, new QTableWidgetItem(query.value("id").toString()));
        table->setItem(i,1, new QTableWidgetItem(query.value("question").toString()));
        ++i;
    }

    //Table Options
    QStringList headerList;
    headerList << tr("id") << tr("Question Text");
    table->setHorizontalHeaderLabels(headerList);
    table->verticalHeader()->setVisible(false);
    table->hideColumn(0);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //table->resizeColumnsToContents();

    db.close();
    return table;
}
// Ab hier
bool Question::updateQuestion(const int id, const QString qTxt, const QString hTxt,
                              const QString sTxt, const QStringList& kwList)
{
    QList<int> kwIds = getPreparedKeywords(kwList);
    openDatabase();
    QSqlQuery query;
    if(!beginTransaction(query, "updateQuestion")) {
        db.close();
        return false;
    }

    query.prepare(tr("UPDATE Questions SET question = :q , hint = :h , answer = :a WHERE id = :id"));
    query.bindValue(tr(":q"),qTxt);
    query.bindValue(tr(":h"),hTxt);
    query.bindValue(tr(":a"),sTxt);
    query.bindValue(tr(":id"),id);
    bool success = query.exec();

    query.prepare("DELETE FROM QuestionKeywords WHERE questionId = :id");
    query.bindValue(":id", id);

    success = success && queryExec(query, "updateQuestion");

    QString queryStr = QString("INSERT INTO QuestionKeywords(questionId, keywordId) "
                               "VALUES ( %1 , ").arg(id);
    for(int i=0; i<kwIds.size()-1; ++i) {
        queryStr.append(QString(" %1 ), ( %2 , ").arg(kwIds.at(i)).arg(id));
    }
    queryStr.append(QString(" %1 );").arg(kwIds.at(kwIds.size()-1)));

    success = success && queryExec(query, "updateQuestion", queryStr);

    if(success) {
        if(!commit(query, "updateQuestion")) {
            db.close();
            return false;
        }
        db.close();
        return true;
    }
    else {
        rollback(query, "updateQuestion");
        db.close();
        return false;
    }

}


// Database methods
bool Question::beginTransaction(QSqlQuery& query, QString method) {
    if(!db.isOpen()) {
        if(!openDatabase()) {
            qDebug() << method << ": could not open DB";
            return false;
        }
    }

    // Begin Transaction automatically turns auto commit off. Rollback/Commit
    // turns it back on automatically.
    if(!query.exec("BEGIN TRANSACTION;")) {
        qDebug() << method << ": Begin transaction failed." << query.lastError();
        db.close();
        return false;
    }
    qDebug() << method << ": Begin transaction successful.";
    return true;
}

bool Question::commit(QSqlQuery& query, QString method) {
    if(!db.isOpen()) {
        if(!openDatabase()) {
            qDebug() << method << ": could not open DB";
            return false;
        }
    }

    if(!query.exec("COMMIT;")) {
        qDebug() << method << ": commit failed." << query.lastError();
        db.close();
        return false;
    }
    qDebug() << method << ": commit successful.";
    return true;
}

bool Question::rollback(QSqlQuery& query, QString method) {
    if(!db.isOpen()) {
        if(!openDatabase()) {
            qDebug() << method << ": could not open DB";
            return false;
        }
    }

    if(!query.exec("ROLLBACK;")) {
        qDebug() << method << ": Rollback failed." << query.lastError();
        db.close();
        return false;
    }
    qDebug() << method << ": Rollback successful.";
    return true;
}

bool Question::queryExec(QSqlQuery& query, QString method, QString stmt) {
    if(!db.isOpen()) {
        if(!openDatabase()) {
            qDebug() << method << ": could not open DB";
            return false;
        }
    }

    bool success;
    if(stmt.isEmpty()) success = query.exec();
    else success = query.exec(stmt);
    if(!success) {
        qDebug() << method << ": query execution failed.\nStmt: "
                 << query.lastQuery() << "\nError: " << query.lastError();
        db.close();
        return false;
    }
    qDebug() << method << ": Query successfully executed.";
    return true;
}

