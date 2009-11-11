#ifndef INTERFACESTABWIDGET_H
#define INTERFACESTABWIDGET_H

#include "../../config.h"
#include "global.h"
#include "utils.h"
#include "utils_no_qt.h"
#include "platforms.h"

#include <QtGui/QTabWidget>
#include <QtGui/QToolButton>
#include "fwbuilder/Interface.h"
#include "fwbuilder/FWObjectDatabase.h"
#include "AddressEditor.h"
#include "InterfaceEditorWidget.h"

namespace Ui {
    class InterfacesTabWidget;
}

struct EditedInterfaceData
{
    QString name;
    QString label;
    QString comment;
    QMap<libfwbuilder::Address*, AddressInfo > addresses;
};

class InterfacesTabWidget : public QTabWidget {
    Q_OBJECT
public:
    InterfacesTabWidget(QWidget *parent = 0);
    ~InterfacesTabWidget();
    QMap<libfwbuilder::Interface*, EditedInterfaceData> getData();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::InterfacesTabWidget *m_ui;
    QToolButton *newInterface;
    libfwbuilder::FWObject *currentTemplate;

public slots:
    void addInterface(libfwbuilder::Interface*);
    void clear();
    void addNewInterface();
    void setTemplate(libfwbuilder::FWObject*);
    void closeTab(int);

};

#endif // INTERFACESTABWIDGET_H
