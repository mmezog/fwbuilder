/* 

                          Firewall Builder

                 Copyright (C) 2004 NetCitadel, LLC

  Author:  Vadim Kurland     vadim@fwbuilder.org

  $Id$

  This program is free software which we release under the GNU General Public
  License. You may redistribute and/or modify this program under the terms
  of that license as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  To get a copy of the GNU General Public License, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#ifndef __IPTADVANCEDDIALOG_H_
#define __IPTADVANCEDDIALOG_H_

#include <ui_iptadvanceddialog_q.h>
#include "DialogData.h"

namespace libfwbuilder {
    class FWObject;
};

class iptAdvancedDialog : public QDialog
{
    Q_OBJECT

    libfwbuilder::FWObject *obj;
    DialogData data;
    Ui::iptAdvancedDialog_q *m_dialog;

 public:
    iptAdvancedDialog(QWidget *parent,libfwbuilder::FWObject *o);
    ~iptAdvancedDialog();

protected slots:

    virtual void accept();
    virtual void reject();
    virtual void help();

    virtual void editProlog();
    virtual void editEpilog();


public slots:
    virtual void switchLOG_ULOG();

//private slots:
//    void on_useNFLOG_toggled(bool checked);
//    void on_useLOG_toggled(bool checked);
};

#endif // __IPTADVANCEDDIALOG_H

