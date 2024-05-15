#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QGridLayout>
#include <QDialogButtonBox>


/************************************************************************
 * This class implements saving, changing and loading of settings.      *
 ************************************************************************/


class Properties : public QDialog
{
    Q_OBJECT

    friend class MainWindow;
private:
    QLineEdit *texLine;
    QLabel *texLab,*sizeLab,*timeOutLab;
    QPushButton *save,*discard,*selectFolder;
    QDialogButtonBox *dialButBox;
    QGridLayout *lay;
    QComboBox *sizeCombo;
    QSpinBox *timeOut;
    int height;
    int width;
    QString pathToTex;
    int time;
    const QString saveFileName = "settings.ini";

    Properties(QWidget *parent = 0);
public:
    //Properties(QWidget *parent = 0);
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    QString getPathToTex() const { return pathToTex; }

    //void setTimeOut(int i) { timeOut->setValue(i); }
    int getTimeOut() { return time; }

public slots:
    void pathDialog();
    void saveProperties();
    void loadProperties();
};

#endif // PROPERTIES_H
