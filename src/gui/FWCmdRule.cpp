/*

                          Firewall Builder

                 Copyright (C) 2009 NetCitadel, LLC

  Author:  Illiya Yalovoy <yalovoy@gmail.com>

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

#include "global.h"

#include "FWWindow.h"
#include "FWCmdRule.h"

#include "events.h"

#include "fwbuilder/FWObjectDatabase.h"
#include "fwbuilder/Policy.h"
#include "fwbuilder/NAT.h"
#include "fwbuilder/Routing.h"
#include "fwbuilder/RuleElement.h"

#include <QDebug>

using namespace libfwbuilder;

/********************************************************
 * FWCmdRule
 ********************************************************/

FWCmdRule::FWCmdRule(ProjectPanel *project, libfwbuilder::RuleSet* ruleset) : FWCmdBasic(project)
{
    this->ruleset = ruleset;
}

RuleSetView* FWCmdRule::getRuleSetView()
{
    RuleSet* crs = project->getCurrentRuleSet();
    if (crs != ruleset) project->openRuleSet(ruleset, true);
    return project->getCurrentRuleSetView();
}

RuleSetModel* FWCmdRule::getRuleSetModel()
{
    RuleSetView* rsv = getRuleSetView();
    return (rsv != NULL)?(RuleSetModel*)getRuleSetView()->model():NULL;
}

void FWCmdRule::notify()
{
    QCoreApplication::postEvent(
        mw, new dataModifiedEvent(project->getFileName(), ruleset->getId()));
}

void FWCmdRule::redo()
{
    RuleSetModel* md = getRuleSetModel();
    redoOnModel(md);
    notify();
}

void FWCmdRule::undo()
{
    RuleSetModel* md = getRuleSetModel();
    undoOnModel(md);
    notify();
}

/********************************************************
 * FWCmdRuleInsert
 ********************************************************/

FWCmdRuleInsert::FWCmdRuleInsert(ProjectPanel *project, RuleSet* ruleset, int position, bool isAfter, Rule* ruleToInsert):
        FWCmdRule(project, ruleset)
{
    this->position = position;
    this->isAfter = isAfter;
    this->ruleToInsert = ruleToInsert;
    this->insertedRule = 0;

    setText(QObject::tr("insert rule"));
}

void FWCmdRuleInsert::redoOnModel(RuleSetModel *md)
{
    if (ruleToInsert == 0)
    {
        if (position == 0)
        {
            insertedRule = md->insertNewRule();
        } else
        {
            QModelIndex index = md->indexForPosition(position);
            insertedRule = md->insertNewRule(index, isAfter);
        }
    } else {
        //TODO: insert ruleToInsert into the ruleset.
    }

    getRuleSetView()->selectRE(insertedRule,0);

}

void FWCmdRuleInsert::undoOnModel(RuleSetModel *md)
{
    QModelIndex index = md->index(insertedRule);
    getRuleSetView()->scrollTo(index,  QAbstractItemView::PositionAtCenter);
    getRuleSetView()->unselect();
    md->removeRow(index.row(), index.parent());
    // do we need delete insertedRule?
    insertedRule = 0;
}