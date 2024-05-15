#include "addeditquestion.h"
#include<QPushButton>
#include<QMessageBox>
#include<QComboBox>
#include<QLineEdit>
#include<QDialogButtonBox>
#include<QHeaderView>


AddEditQuestion::AddEditQuestion(std::shared_ptr<Question> q, std::shared_ptr<Keyword> k, QDialog *parent)
    : QDialog(parent), questions(q), kw(k) {
    // Add/load/edit a question (GUI and Logic)

    saveBut = new QPushButton(QIcon(tr("./ico/save.ico")),tr("Save"), this);
    quitBut = new QPushButton(tr("Quit"), this);
    searchBut = new QPushButton(tr("Search & load"), this);
    addBut = new QPushButton(tr("Add\n Keyword"), this);
    selBut = new QPushButton(tr("Select\n Keywords"), this);
    delBut = new QPushButton(tr("Delete\n Keyword"), this);
    delQuestBut = new QPushButton(tr("Delete"), this);

    questionLab = new QLabel(tr("Question: "), this);
    hintLab = new QLabel(tr("Hint: "), this);
    answerLab = new QLabel(tr("Answer: "), this);

    keywordList = new QListWidget(this);
    selectedList = new QListWidget(this);

    quest = new QTextEdit(this);
    ans = new QTextEdit(this);
    hint = new QTextEdit(this);

    dialogLay = new QGridLayout(this);


    //Initialisation
    id = 0;
    resetKwLists();
    updateAddEditLists();
    questionLoaded = false;
    delQuestion = false;
    delQuestBut->setVisible(false);

    keywordList->setFixedSize(140,120);
    selectedList->setFixedSize(140,120);

    quest->setFixedHeight(60);
    ans->setFixedHeight(60);
    hint->setFixedHeight(60);

    //Layout
    dialogLay->addWidget(saveBut,0,0,1,1);
    dialogLay->addWidget(searchBut,0,1,1,1);
    dialogLay->addWidget(delQuestBut,0,2,1,1);
    dialogLay->addWidget(quitBut,0,3,1,1);

    dialogLay->addWidget(keywordList,1,0,3,2);
    dialogLay->addWidget(selectedList,1,2,3,2);
    dialogLay->addWidget(addBut,1,4,1,1);
    dialogLay->addWidget(selBut,2,4,1,1);
    dialogLay->addWidget(delBut,3,4,1,1);

    dialogLay->addWidget(questionLab,5,0,1,2);
    dialogLay->addWidget(quest,6,0,2,5);
    dialogLay->addWidget(answerLab,9,0,1,2);
    dialogLay->addWidget(ans,10,0,3,5);
    dialogLay->addWidget(hintLab,13,0,1,2);
    dialogLay->addWidget(hint,14,0,2,5);

    this->setLayout(dialogLay);

    //Connection
    connect(keywordList,SIGNAL(clicked(QModelIndex)), this,SLOT(dbToSel_addEdit(QModelIndex)));
    connect(selectedList,SIGNAL(clicked(QModelIndex)), this,SLOT(selToDB_addEdit(QModelIndex)));
    connect(searchBut,SIGNAL(clicked()), this,SLOT(setLoadedQuestionTrue()));
    connect(searchBut,SIGNAL(clicked()), this,SLOT(accept()));
    connect(delQuestBut,SIGNAL(clicked()), this,SLOT(flipDelQuestion()));
    connect(delQuestBut,SIGNAL(clicked()), this,SLOT(accept()));

    connect(addBut,SIGNAL(clicked()), this,SLOT(addKeyword()));
    connect(selBut,SIGNAL(clicked()), this,SLOT(selectKeywords()));
    connect(delBut,SIGNAL(clicked()), this,SLOT(deleteKeyword()));
    connect(quitBut,SIGNAL(clicked()), this,SLOT(close()));
    connect(saveBut,SIGNAL(clicked()), this,SLOT(accept()));

    connect(this, SIGNAL(finished(int)), this, SLOT(processResult(int)));


}

void AddEditQuestion::open() {
    // Reset lists
    quest->clear();
    ans->clear();
    hint->clear();
    resetKwLists();
    QDialog::open();
}

void AddEditQuestion::processResult(int result) {
    //Resultmagic
    if(result!=QDialog::Accepted) {
        id = -1;
        questionLoaded = false;
        delQuestion = false;
        resetKwLists();
        return;
    }
    if( questionLoaded ) { // Fill QDialog Widgets with values
        int res = this->searchQuestion();
        if(res) {
            id = questions.get()->getId();
            quest->setText(questions.get()->getQuestion());
            hint->setText(questions.get()->getHint());
            ans->setText(questions.get()->getSolution());
            selectedKeywords = questions.get()->getKwList();  // Kws of question
            selectedKeywords.sort(Qt::CaseInsensitive);
            selectedList->clear();
            selectedList->addItems(selectedKeywords);
            keywordList->clear();
            // Add Kws of question to current Kws temporarily.
            QStringList helpList = dbKeywords;
            minusList(&helpList, &selectedKeywords);
            currentKeywords.clear();
            currentKeywords.append(helpList);
            currentKeywords.sort(Qt::CaseInsensitive);
            keywordList->addItems(currentKeywords);

            delQuestBut->setVisible(true);
            questionLoaded = false;
        }

    }
    else if(delQuestion) {
        //Frage löschen per id
        if(!questions.get()->deleteQuestion(id)) {
            QMessageBox::critical(0,tr("Delete Question"),
                                  tr("Question could not be deleted!"),QMessageBox::Ok);
        }
        quest->clear();
        hint->clear();
        ans->clear();
        currentKeywords = dbKeywords;
        selectedKeywords.clear();
        keywordList->clear();
        keywordList->addItems(currentKeywords);
        selectedList->clear();
        delQuestBut->setVisible(false);
        id = -1;
        delQuestion = false;
    }

    else if((selectedList->count()==0) || (quest->toPlainText().isEmpty())
             || hint->toPlainText().isEmpty() || ans->toPlainText().isEmpty())
    {
        // If a essential field is left empty
        QMessageBox::critical(nullptr, tr("Save/Edit Questions"),
                              tr("Please fill in <u><b>all</b></u> fields!"),
                              QMessageBox::Ok);

    }
    else if(questions.get()->idExists(id)==1) {
        // Update the question and do not save it!
        // I do not need to check if all fields are filled, since the elseif
        // case above does it.
        bool res = questions.get()->updateQuestion(id, quest->toPlainText(), hint->toPlainText(),
                                                   ans->toPlainText(), selectedKeywords);
        if(res==true) { // Question was successfully updated -> empty fields.
            QMessageBox::information(nullptr, tr("Update Question"),
                                     tr("Question successfully updated!"), QMessageBox::Ok);
            quest->clear();
            hint->clear();
            ans->clear();
            currentKeywords = dbKeywords;
            selectedKeywords.clear();
            keywordList->clear();
            keywordList->addItems(currentKeywords);
            selectedList->clear();
            delQuestBut->setVisible(false);
            id = -1;
            questionLoaded = false;
        }
        else{
            QMessageBox::critical(nullptr, tr("Update Question"),
                                  tr("Question could not be updated!"),QMessageBox::Ok);
        }
    }
    else if(questions.get()->idExists(id)==0) { // save question
        bool s = questions.get()->addQuestion(selectedKeywords, quest->toPlainText(),
                                              hint->toPlainText(),ans->toPlainText());
        if(!s) {
            QMessageBox::critical(nullptr ,tr("Add Question"),
                                  tr("Question could not be added!"),
                                  QMessageBox::Ok);
        }
        else { //Textboxen leeren
            quest->clear();
            hint->clear();
            ans->clear();
        }
    }
    updateAddEditLists();
    QDialog::open();


}

void AddEditQuestion::resetKwLists() {
    dbKeywords = questions.get()->getCurrentKws();
    currentKeywords = dbKeywords;
    selectedKeywords.clear();
    updateAddEditLists();
}

void AddEditQuestion::resetKwLists(QStringList& dbKws, QStringList& curKws, QStringList& selKws) {
    resetKwLists();
    dbKws = dbKeywords;
    curKws = currentKeywords;
    selKws.clear();
}

void AddEditQuestion::updateAddEditLists() {
    // Updates the ListWidgets (keywordList and selectedList) in AddEdit Dialog.

    this->keywordList->clear();
    this->selectedList->clear();
    currentKeywords = dbKeywords;
    minusList(&currentKeywords, &selectedKeywords);
    this->keywordList->addItems(currentKeywords);
    this->selectedList->addItems(selectedKeywords);
}

void AddEditQuestion::minusList(QStringList *all, QStringList *cur) {
    //Deletes entries from List all, that are contained in cur
    for(int i = 0; i<cur->size();i++) {
        all->removeOne(cur->at(i));
    }
}

int AddEditQuestion::searchQuestion() {
    tab = std::make_unique<QTableWidget>();

    auto dial = std::make_unique<QDialog>();
    auto combo = new QComboBox(dial.get());
    auto searchLine = new QLineEdit(dial.get());
    auto searchBut = new QPushButton(tr("Search"),dial.get());searchBut->setVisible(false);
    auto cancelBut = new QPushButton(tr("Cancel"),dial.get());
    auto dialButBox = new QDialogButtonBox(dial.get());
    auto gridLay = new QGridLayout(dial.get());


    QStringList comboItems;
    comboItems << tr("number") << tr("Question-text") << tr("Box-Nr");
    combo->addItems(comboItems);

    dialButBox->addButton(searchBut,QDialogButtonBox::AcceptRole);
    dialButBox->addButton(cancelBut,QDialogButtonBox::RejectRole);


    connect(searchBut,SIGNAL(clicked()),dial.get(),SLOT(accept()));
    connect(cancelBut,SIGNAL(clicked()),dial.get(),SLOT(reject()));


    gridLay->addWidget(combo,     1,1,1,1);
    gridLay->addWidget(searchLine,1,2,1,2);
    gridLay->addWidget(dialButBox,6,1,1,3,Qt::AlignRight);
    dial.get()->setLayout(gridLay);

    int result;
    bool finish = false;
    do{
        result = dial.get()->exec();
        if(result == QDialog::Accepted) {
            if(!tab.get()->selectedItems().isEmpty()) { //Element was chosen
                this->evaluateItem(tab.get()->currentRow(), tab.get()->currentColumn());
                tab.reset(nullptr);
                return 1;
            }
            else { //Search Button was cklicked
                tab = questions.get()->searchQuestion(combo->currentText(), searchLine->text());

                connect(tab.get(),SIGNAL(cellDoubleClicked(int,int)),
                        dial.get(),SLOT(accept()));

                //tab->horizontalHeader()->setStretchLastSection(true);
                tab.get()->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

                //tab->resizeColumnsToContents();
                if(tab.get()->columnWidth(1)>600) tab.get()->setFixedWidth(600);
                else if(tab.get()->columnWidth(1)==0) tab.get()->setFixedSize(254,10);
                else if(tab.get()->columnWidth(1)<254) {
                    tab.get()->setColumnWidth(1,254);
                    tab.get()->setFixedWidth(254);
                }
                else { tab.get()->setFixedWidth(tab.get()->columnWidth(1)); }

                gridLay->addWidget(tab.get(),2,1,4,3,Qt::AlignHCenter);
                dial.get()->setLayout(gridLay);
            }
        }
        else { //Cancel was clicked
            finish = true;
        }

    }
    while(!finish);
    tab.reset(nullptr);
    return 0;

}

void AddEditQuestion::evaluateItem(int r, int c) {
    // In Add/Edit Questions, when a question should be loaded. The search
    // result is a table and double click on this table calls this method.

    questions.get()->loadQuestion(tab.get()->item(r,c-1)->text().toInt());
    qDebug() << "MainWindow::evaluateItem: " << r << " " << c-1 << " "
             << tab.get()->item(r,0)->text().toInt();
}

void AddEditQuestion::dbToSel_addEdit(const QModelIndex& mod) {
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

void AddEditQuestion::selToDB_addEdit(const QModelIndex& mod) {
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

void AddEditQuestion::removeItemFromAllLists(const QString kw) {
    selectedKeywords.removeOne(kw);
    dbKeywords.removeOne(kw);
    currentKeywords.removeOne(kw);
}

void AddEditQuestion::addKeyword() {
    if(!kw.get()->addKeyword()) return;
    dbKeywords = questions->getCurrentKws();
    updateAddEditLists();
}

void AddEditQuestion::selectKeywords() {
    kw.get()->selectKeywords(dbKeywords);
    updateAddEditLists();
}

void AddEditQuestion::deleteKeyword() {
    const QString result = kw.get()->deleteKeyword();
    if(result=="") return;
    removeItemFromAllLists(result);
    updateAddEditLists();
}
