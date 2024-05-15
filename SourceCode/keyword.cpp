#include "keyword.h"

#include<QDialog>
#include<QDialogButtonBox>
#include<QPushButton>
#include<QLabel>
#include<QGridLayout>
#include<QLineEdit>
#include<QMessageBox>
#include<QComboBox>
#include<memory>

Keyword::Keyword(std::shared_ptr<Question> q, QObject *parent)
    : QObject(parent), questions(q) {

    dbKeywords = q.get()->getKeywordsFromDb();

}

bool Keyword::addKeyword() {
    //Adds a new Keyword to Database (GUI and Logic)

    auto dialog = std::make_unique<QDialog>();
    QDialogButtonBox *dialButBox = new QDialogButtonBox(dialog.get());
    QPushButton *ok = new QPushButton(tr("Save"), dialog.get());
    QPushButton *cancel = new QPushButton(tr("Cancel"),dialog.get());
    QLabel *name = new QLabel(tr("Keyword: "), dialog.get());
    QLineEdit *nameLine = new QLineEdit(dialog.get());
    QGridLayout *dialogLay = new QGridLayout(dialog.get());

    //Initialisation
    dialButBox->addButton(ok, QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancel, QDialogButtonBox::RejectRole);

    //Connection
    connect(ok, SIGNAL(clicked()), dialog.get(), SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), dialog.get(), SLOT(reject()));

    //Layout
    dialogLay->addWidget(name,      1,0,1,1);
    dialogLay->addWidget(nameLine,  1,1,1,2);
    dialogLay->addWidget(dialButBox,2,0,1,3);

    dialog.get()->setLayout(dialogLay);
    dialog.get()->setWindowTitle(tr("Add Keyword"));

    //Results
    int result = dialog.get()->exec();

    if(result != QDialog::Accepted) return false;

    //Test whether keyword already exists
    if(questions.get()->getKeywordsFromDb().contains(nameLine->text())) {
        QMessageBox::critical(dialog.get(), tr("Add Keyword"),
                              tr("Keyword already exists"), QMessageBox::Ok);
        return false;
    }

    //Save the new Keyword
    const QString newKw = nameLine->text();
    if(!questions.get()->writeKeyword(newKw)) {
        QMessageBox::critical(dialog.get(), tr("Add Keyword"),
                              tr("Keyword could not be added!"), QMessageBox::Ok);
        return false;
    }

    //Update Stringlists
    dbKeywords << newKw;
    dbKeywords.sort(Qt::CaseSensitive);

    return true;

}

const QString Keyword::deleteKeyword() {
    //Deletes a Keyword from Database (GUI and Logic)

    auto dialog = std::make_unique<QDialog>();
    QDialogButtonBox *dialButBox = new QDialogButtonBox(dialog.get());
    QLabel *type = new QLabel(tr("Type: "),dialog.get());
    QPushButton *del = new QPushButton(tr("Delete"),dialog.get());
    QPushButton *cancel = new QPushButton(tr("Cancel"),dialog.get());
    QComboBox *typeBox = new QComboBox(dialog.get());
    QGridLayout *dialogLay = new QGridLayout(dialog.get());

    //Initialisation
    dialButBox->addButton(del,QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancel,QDialogButtonBox::RejectRole);

    QStringList strList = questions.get()->getKeywordsFromDb();  // We need all Kws here!
    typeBox->addItems(strList);

    //connects
    connect(del,SIGNAL(clicked()),dialog.get(),SLOT(accept()));
    connect(cancel,SIGNAL(clicked()),dialog.get(),SLOT(reject()));

    //Layout
    dialogLay->addWidget(type,      0,0,1,1);
    dialogLay->addWidget(typeBox,   0,1,1,2);
    dialogLay->addWidget(dialButBox,1,0,1,3);

    dialog->setLayout(dialogLay);

    int result = dialog.get()->exec();

    //result
    if(result != QDialog::Accepted) return "";

    const QString wordToDelete = typeBox->currentText();
    bool s = questions.get()->deleteKeyword(wordToDelete);
    if(!s) {
        QMessageBox::critical(nullptr,tr("Database Error"),
                              tr("Error! Keyword could not be deleted!"),
                              QMessageBox::Ok);
        return "";
    }

    this->removeItemFromAllLists(wordToDelete);

    return wordToDelete;

}

void Keyword::removeItemFromAllLists(const QString kw) {
    selectedKeywords.removeOne(kw);
    dbKeywords.removeOne(kw);
}

void Keyword::selectKeywords(QStringList& newSelKws) {
    // Learn only Questions with selected keywords.

    auto dialog = std::make_unique<QDialog>();
    QLabel *keywordTxt = new QLabel(tr("Only Question with following\n Keywords:"));
    QPushButton *start = new QPushButton(tr("Select"), dialog.get());
    QPushButton *cancel = new QPushButton(tr("Cancel"), dialog.get());
    QGridLayout *gridLay = new QGridLayout(dialog.get());
    QDialogButtonBox *dialButBox = new QDialogButtonBox(dialog.get());
    keywordList = new QListWidget(dialog.get());
    keywordList->setFixedSize(140,100);
    selectedList = new QListWidget(dialog.get());
    selectedList->setFixedSize(140,100);

    // Fill the values from the DB in both lists, such that the selected ones are
    // in the list on the right side and all other in the list on the left side.
    dbKeywords = questions.get()->getKeywordsFromDb();
    bool noCurrentKws;
    selectedKeywords = questions.get()->getCurrentKws(&noCurrentKws);
    if(noCurrentKws) selectedKeywords.clear();

    minusList(&dbKeywords, &selectedKeywords);

    selectedList->addItems(selectedKeywords);
    keywordList->addItems(dbKeywords);

    dialButBox->addButton(start,QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancel,QDialogButtonBox::RejectRole);

    gridLay->addWidget(keywordTxt,   0,2,1,2);
    gridLay->addWidget(keywordList,  1,0,1,2);
    gridLay->addWidget(selectedList, 1,2,1,2);
    gridLay->addWidget(dialButBox,   2,0,1,4, Qt::AlignRight);

    dialog.get()->setLayout(gridLay);

    connect(keywordList,SIGNAL(clicked(QModelIndex)),
            this,SLOT(dbToSel(QModelIndex)));
    connect(selectedList,SIGNAL(clicked(QModelIndex)),
            this,SLOT(selToDB(QModelIndex)));
    connect(start,SIGNAL(clicked()),dialog.get(),SLOT(accept()));
    connect(cancel,SIGNAL(clicked()),dialog.get(),SLOT(reject()));

    int ret = dialog.get()->exec();
    if(ret==QDialog::Rejected) return;

    // Update current Kws
    questions.get()->saveCurrentKw(selectedKeywords);
    newSelKws = selectedKeywords;

}

void Keyword::dbToSel(const QModelIndex& mod) {
    //Swaps a keyword from dbKeywordlist to selectionList in method selectKeywords
    this->selectedKeywords.append(mod.data().toString());
    this->selectedKeywords.sort(Qt::CaseInsensitive);
    this->dbKeywords.removeAll(mod.data().toString());

    //Listen aktualisieren
    this->keywordList->clear();
    this->keywordList->addItems(dbKeywords);
    this->selectedList->clear();
    this->selectedList->addItems(selectedKeywords);
}

void Keyword::selToDB(const QModelIndex& mod) {
    //Swaps a keyword from selection list to dbKeyword list in method selectKeywords
    dbKeywords.append(mod.data().toString());
    dbKeywords.sort(Qt::CaseInsensitive);
    selectedKeywords.removeAll(mod.data().toString());

    //Listen aktualisieren
    this->keywordList->clear();
    this->keywordList->addItems(dbKeywords);
    this->selectedList->clear();
    this->selectedList->addItems(selectedKeywords);
}

void Keyword::minusList(QStringList *all, QStringList *cur) {
    //Deletes entries from List all, that are contained in cur
    for(int i = 0; i<cur->size();i++) {
        all->removeOne(cur->at(i));
    }
}
