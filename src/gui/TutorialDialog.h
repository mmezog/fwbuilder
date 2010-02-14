#ifndef TUTORIALDIALOG_H
#define TUTORIALDIALOG_H

#include <QDialog>
#include "TutorialAnimator.h"


namespace Ui {
    class TutorialDialog_q;
}

class TutorialDialog : public QDialog {
    Q_OBJECT
public:
    TutorialDialog(QWidget *parent = 0);
    ~TutorialDialog();

    QString getScenarioForPage(int page);

    void resizeEvent(QResizeEvent *);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TutorialDialog_q *ui;
    int currentPage;
    TutorialAnimator *animator;

public slots:
    void previous();
    void next();
    void reset();
    void demonstrate();
    void showPage(int page);
};

#endif // TUTORIALDIALOG_H