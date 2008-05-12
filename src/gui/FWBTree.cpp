/*

                          Firewall Builder

                 Copyright (C) 2003 NetCitadel, LLC

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

#include "fwbuilder_ph.h"

#include "config.h"
#include "global.h"

#include <qobject.h>
#include <qmessagebox.h>

#include "FWBTree.h"

#include "fwbuilder/FWObjectDatabase.h"

#include "fwbuilder/Library.h"
#include "fwbuilder/Firewall.h"
#include "fwbuilder/Host.h"
#include "fwbuilder/Network.h"
#include "fwbuilder/IPv4.h"
#include "fwbuilder/DNSName.h"
#include "fwbuilder/AddressTable.h"
#include "fwbuilder/AddressRange.h"
#include "fwbuilder/ObjectGroup.h"
#include "fwbuilder/Interface.h"
#include "fwbuilder/CustomService.h"
#include "fwbuilder/IPService.h"
#include "fwbuilder/ICMPService.h"
#include "fwbuilder/TCPService.h"
#include "fwbuilder/UDPService.h"
#include "fwbuilder/ServiceGroup.h"
#include "fwbuilder/Interval.h"
#include "fwbuilder/IntervalGroup.h"
#include <fwbuilder/TagService.h>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace libfwbuilder;

const char* systemObjects[] = {
    "Objects",
    "Objects/Addresses",
    "Objects/DNS Names",
    "Objects/Address Tables",
    "Objects/Address Ranges",
    "Objects/Groups",
    "Objects/Hosts",
    "Objects/Networks",

    "Services",
    "Services/Custom",
    "Services/Groups",
    "Services/IP",
    "Services/ICMP",
    "Services/TCP",
    "Services/UDP",
    "Services/TagServices",
    "Services/User",

    "Firewalls",

    "Time",

    NULL
};

map<string,bool> standardIDs;

FWBTree::FWBTree()
{
    systemGroupPaths[Library::TYPENAME]       = "";

    systemGroupPaths[IPv4::TYPENAME]          = "Objects/Addresses";
    systemGroupPaths[DNSName::TYPENAME]       = "Objects/DNS Names";
    systemGroupPaths[AddressTable::TYPENAME]  = "Objects/Address Tables";
    systemGroupPaths[AddressRange::TYPENAME]  = "Objects/Address Ranges";
    systemGroupPaths[ObjectGroup::TYPENAME]   = "Objects/Groups";
    systemGroupPaths[Host::TYPENAME]          = "Objects/Hosts";
    systemGroupPaths[Network::TYPENAME]       = "Objects/Networks";

    systemGroupPaths[ServiceGroup::TYPENAME]  = "Services/Groups";
    systemGroupPaths[CustomService::TYPENAME] = "Services/Custom";
    systemGroupPaths[IPService::TYPENAME]     = "Services/IP";
    systemGroupPaths[ICMPService::TYPENAME]   = "Services/ICMP";
    systemGroupPaths[TCPService::TYPENAME]    = "Services/TCP";
    systemGroupPaths[UDPService::TYPENAME]    = "Services/UDP";
    systemGroupPaths[TagService::TYPENAME]          = "Services/TagServices";

    systemGroupPaths[Firewall::TYPENAME]      = "Firewalls";

    systemGroupPaths[Interval::TYPENAME]      = "Time";



    systemGroupTypes[Firewall::TYPENAME]=      ObjectGroup::TYPENAME;
    systemGroupNames[Firewall::TYPENAME]=      "Firewalls"    ;

    systemGroupTypes[Host::TYPENAME]=          ObjectGroup::TYPENAME;
    systemGroupNames[Host::TYPENAME]=          "Hosts"          ;

    systemGroupTypes[Network::TYPENAME]=       ObjectGroup::TYPENAME;
    systemGroupNames[Network::TYPENAME]=       "Networks"       ;

    systemGroupTypes[IPv4::TYPENAME]=          ObjectGroup::TYPENAME;
    systemGroupNames[IPv4::TYPENAME]=          "Addresses"      ;

    systemGroupTypes[DNSName::TYPENAME]=       ObjectGroup::TYPENAME;
    systemGroupNames[DNSName::TYPENAME]=       "DNS Names"      ;

    systemGroupTypes[AddressTable::TYPENAME]=  ObjectGroup::TYPENAME;
    systemGroupNames[AddressTable::TYPENAME]=  "Address Tables"      ;

    systemGroupTypes[AddressRange::TYPENAME]=  ObjectGroup::TYPENAME;
    systemGroupNames[AddressRange::TYPENAME]=  "Address Ranges" ;

    systemGroupTypes[ObjectGroup::TYPENAME]=   ObjectGroup::TYPENAME;
    systemGroupNames[ObjectGroup::TYPENAME]=   "Groups"         ;

    systemGroupTypes[CustomService::TYPENAME]= ServiceGroup::TYPENAME;
    systemGroupNames[CustomService::TYPENAME]= "Custom";

    systemGroupTypes[IPService::TYPENAME]=     ServiceGroup::TYPENAME;
    systemGroupNames[IPService::TYPENAME]=     "IP"            ;

    systemGroupTypes[ICMPService::TYPENAME]=   ServiceGroup::TYPENAME;
    systemGroupNames[ICMPService::TYPENAME]=   "ICMP"          ;

    systemGroupTypes[TCPService::TYPENAME]=    ServiceGroup::TYPENAME;
    systemGroupNames[TCPService::TYPENAME]=    "TCP"           ;

    systemGroupTypes[UDPService::TYPENAME]=    ServiceGroup::TYPENAME;
    systemGroupNames[UDPService::TYPENAME]=    "UDP"           ;

    systemGroupTypes[TagService::TYPENAME]=          ServiceGroup::TYPENAME;
    systemGroupNames[TagService::TYPENAME]=          "TagService"           ;

    systemGroupTypes[ServiceGroup::TYPENAME]=  ServiceGroup::TYPENAME;
    systemGroupNames[ServiceGroup::TYPENAME]=  "Groups"        ;

    systemGroupTypes[Interval::TYPENAME]=      IntervalGroup::TYPENAME;
    systemGroupNames[Interval::TYPENAME]=      "Time"         ;

    systemGroupTypes[Interface::TYPENAME]=     "";
    systemGroupNames[Interface::TYPENAME]=     "";

    systemGroupTypes[Library::TYPENAME]=      FWObjectDatabase::TYPENAME;
    systemGroupNames[Library::TYPENAME]=      "FWObjectDatabase";

    standardIDs["syslib000"]=true;
    standardIDs["syslib001"]=true;

    standardIDs["sysid0"]   =true;
    standardIDs["sysid1"]   =true;
    standardIDs["sysid2"]   =true;
    standardIDs["sysid99"]  =true;

    standardIDs["stdid01"]  =true;
    standardIDs["stdid01_1"]=true;
    standardIDs["stdid02"]  =true;
    standardIDs["stdid02_1"]=true;
    standardIDs["stdid03"]  =true;
    standardIDs["stdid03_1"]=true;
    standardIDs["stdid04"]  =true;
    standardIDs["stdid04_1"]=true;
    standardIDs["stdid05"]  =true;
    standardIDs["stdid05_1"]=true;
    standardIDs["stdid06"]  =true;
    standardIDs["stdid06_1"]=true;
    standardIDs["stdid07"]  =true;
    standardIDs["stdid07_1"]=true;
    standardIDs["stdid08"]  =true;
    standardIDs["stdid08_1"]=true;
    standardIDs["stdid09"]  =true;
    standardIDs["stdid09_1"]=true;
    standardIDs["stdid10"]  =true;
    standardIDs["stdid10_1"]=true;
    standardIDs["stdid11"]  =true;
    standardIDs["stdid11_1"]=true;
    standardIDs["stdid12"]  =true;
    standardIDs["stdid12_1"]=true;
    standardIDs["stdid13"]  =true;
    standardIDs["stdid13_1"]=true;
    standardIDs["stdid14"]  =true;
    standardIDs["stdid14_1"]=true;
    standardIDs["stdid15"]  =true;
    standardIDs["stdid15_1"]=true;
    standardIDs["stdid16"]  =true;
    standardIDs["stdid16_1"]=true;
    standardIDs["stdid17"]  =true;
    standardIDs["stdid17_1"]=true;
    standardIDs["stdid18"]  =true;
    standardIDs["stdid18_1"]=true;
    standardIDs["stdid19"]  =true;
    standardIDs["stdid19_1"]=true;


    copyMenuState[""] = false;
    copyMenuState["Firewalls"] = false;
    copyMenuState["Objects"] = false;
    copyMenuState["Objects/Addresses"] = false;
    copyMenuState["Objects/DNS Names"] = false;
    copyMenuState["Objects/Address Tables"] = false;
    copyMenuState["Objects/Address Ranges"] = false;
    copyMenuState["Objects/Groups"] = false;
    copyMenuState["Objects/Hosts"] = false;
    copyMenuState["Objects/Networks"] = false;
    copyMenuState["Services"] = false;
    copyMenuState["Services/Custom"] = false;
    copyMenuState["Services/Groups"] = false;
    copyMenuState["Services/ICMP"] = false;
    copyMenuState["Services/IP"] = false;
    copyMenuState["Services/TCP"] = false;
    copyMenuState["Services/UDP"] = false;
    copyMenuState["Services/TagServices"] = false;
    copyMenuState["Time"] = false;

    cutMenuState[""] = true;
    cutMenuState["Firewalls"] = false;
    cutMenuState["Objects"] = false;
    cutMenuState["Objects/Addresses"] = false;
    cutMenuState["Objects/DNS Names"] = false;
    cutMenuState["Objects/Address Tables"] = false;
    cutMenuState["Objects/Address Ranges"] = false;
    cutMenuState["Objects/Groups"] = false;
    cutMenuState["Objects/Hosts"] = false;
    cutMenuState["Objects/Networks"] = false;
    cutMenuState["Services"] = false;
    cutMenuState["Services/Custom"] = false;
    cutMenuState["Services/Groups"] = false;
    cutMenuState["Services/ICMP"] = false;
    cutMenuState["Services/IP"] = false;
    cutMenuState["Services/TCP"] = false;
    cutMenuState["Services/UDP"] = false;
    cutMenuState["Services/TagServices"] = false;
    cutMenuState["Time"] = false;

    pasteMenuState[""] = false;
    pasteMenuState["Firewalls"] = true;
    pasteMenuState["Objects"] = false;
    pasteMenuState["Objects/Addresses"] = true;
    pasteMenuState["Objects/DNS Names"] = true;
    pasteMenuState["Objects/Address Tables"] = true;
    pasteMenuState["Objects/Address Ranges"] = true;
    pasteMenuState["Objects/Groups"] = true;
    pasteMenuState["Objects/Hosts"] = true;
    pasteMenuState["Objects/Networks"] = true;
    pasteMenuState["Services"] = false;
    pasteMenuState["Services/Custom"] = true;
    pasteMenuState["Services/Groups"] = true;
    pasteMenuState["Services/ICMP"] = true;
    pasteMenuState["Services/IP"] = true;
    pasteMenuState["Services/TCP"] = true;
    pasteMenuState["Services/UDP"] = true;
    pasteMenuState["Services/TagServices"] = true;
    pasteMenuState["Time"] = true;

    deleteMenuState[""] = true;
    deleteMenuState["Firewalls"] = false;
    deleteMenuState["Objects"] = false;
    deleteMenuState["Objects/Addresses"] = false;
    deleteMenuState["Objects/DNS Names"] = false;
    deleteMenuState["Objects/Address Tables"] = false;
    deleteMenuState["Objects/Address Ranges"] = false;
    deleteMenuState["Objects/Groups"] = false;
    deleteMenuState["Objects/Hosts"] = false;
    deleteMenuState["Objects/Networks"] = false;
    deleteMenuState["Services"] = false;
    deleteMenuState["Services/Custom"] = false;
    deleteMenuState["Services/Groups"] = false;
    deleteMenuState["Services/ICMP"] = false;
    deleteMenuState["Services/IP"] = false;
    deleteMenuState["Services/TCP"] = false;
    deleteMenuState["Services/UDP"] = false;
    deleteMenuState["Services/TagServices"] = false;
    deleteMenuState["Time"] = false;
}

/**
 * returns true if object 'obj' is a system group.  System groups are
 * those that hold other objects. Unlike user-defined groups, system
 * groups always contain only objects themselves and never contain
 * references to objects. User-defined groups, on the other hand,
 * always contain only references to objects.
 *
 */
bool FWBTree::isSystem(FWObject *obj)
{
    if (Library::isA(obj))
        return (obj->getId()==STANDARD_LIB || obj->getId()==DELETED_LIB);

    if (FWObjectDatabase::isA(obj)) return true;

    string path=obj->getPath(true);  // relative path

    for (const char **cptr=systemObjects; *cptr!=NULL; cptr++)
        if (path== *cptr) return true;


    return false;
}

bool FWBTree::isStandardId(FWObject *obj)
{
    return standardIDs[ obj->getId() ];
}

bool FWBTree::validateForInsertion(FWObject *target,FWObject *obj)
{
    if (fwbdebug) qDebug("FWBTree::validateForInsertion  target %s  obj %s",
                         target->getTypeName().c_str(),
                         obj->getTypeName().c_str());

    if (Host::isA(target)      && Interface::isA(obj))   return true;
    if (Firewall::isA(target)  && Interface::isA(obj))   return true;
    if (Interface::isA(target) && IPv4::isA(obj))        return true;
    if (Interface::isA(target) && physAddress::isA(obj)) return true;

    QString parentType = systemGroupTypes[obj->getTypeName().c_str()];
    QString parentName = systemGroupNames[obj->getTypeName().c_str()];

/* parentType or/and parentName are going to be empty if information
 * about object obj is missing in systemGroupTypes/Names tables
 */
    if (parentType.isEmpty() || parentName.isEmpty()) return false;

    if (target->getTypeName() == string(parentType.toLatin1()) &&
	target->getName()     == string(parentName.toLatin1()) )
        return true;

    return false;
}

void FWBTree::getStandardSlotForObject(const QString &objType,
                                       QString &parentType,
                                       QString &parentName)
{
    parentType = systemGroupTypes[objType];
    parentName = systemGroupNames[objType];
}

/**
 * this method finds standard system folder for an object of a given
 * type in a given library. This method implemented our standard tree
 * structure (the one that is created in the method createNewLibrary)
 */
FWObject* FWBTree::getStandardSlotForObject(FWObject* lib,const QString &objType)
{
    QString path = systemGroupPaths[objType];

    if (path.isEmpty()) return lib;

    QString level1 = path.section('/',0,0);
    QString level2 = path.section('/',1,1);

    FWObject::iterator i=std::find_if(lib->begin(),lib->end(),
        FWObjectNameEQPredicate(static_cast<const char*>(level1.toAscii())));
    if (i==lib->end()) return NULL;
    FWObject *l1obj = *i;
    if (level2.isEmpty()) return l1obj;

    i=std::find_if(l1obj->begin(),l1obj->end(),
        FWObjectNameEQPredicate(static_cast<const char*>(level2.toAscii())));
    if (i==l1obj->end()) return NULL;
    return (*i);
}

FWObject* FWBTree::createNewLibrary(FWObjectDatabase *db)
{

    FWObject *nlib = db->create(Library::TYPENAME);
    db->add(nlib);
    nlib->setName( string(QObject::tr("New Library").toUtf8()) );

    FWObject *o1 = db->create(ObjectGroup::TYPENAME);
    o1->setName("Objects");
    nlib->add(o1);

    FWObject *o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Addresses");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("DNS Names");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Address Tables");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Groups");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Hosts");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Networks");
    o1->add(o2);

    o2 = db->create(ObjectGroup::TYPENAME);
    o2->setName("Address Ranges");
    o1->add(o2);

    o1 = db->create(ServiceGroup::TYPENAME);
    o1->setName("Services");
    nlib->add(o1);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("Groups");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("ICMP");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("IP");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("TCP");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("UDP");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("Custom");
    o1->add(o2);

    o2 = db->create(ServiceGroup::TYPENAME);
    o2->setName("TagServices");
    o1->add(o2);

    o1 = db->create(ObjectGroup::TYPENAME);
    o1->setName("Firewalls");
    nlib->add(o1);

    o1 = db->create(IntervalGroup::TYPENAME);
    o1->setName("Time");
    nlib->add(o1);

    return nlib;
}
