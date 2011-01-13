/* 

                          Firewall Builder

                 Copyright (C) 2002 NetCitadel, LLC

  Author:  Vadim Kurland     vadim@vk.crocodile.org

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

#include "config.h"

#include "PolicyCompiler_cisco.h"
#include "NamedObjectsAndGroupsSupport.h"

#include "fwbuilder/FWObjectDatabase.h"
#include "fwbuilder/RuleElement.h"
#include "fwbuilder/IPService.h"
#include "fwbuilder/ICMPService.h"
#include "fwbuilder/TCPService.h"
#include "fwbuilder/UDPService.h"
#include "fwbuilder/Network.h"
#include "fwbuilder/Policy.h"
#include "fwbuilder/Interface.h"
#include "fwbuilder/Management.h"
#include "fwbuilder/Resources.h"
#include "fwbuilder/AddressTable.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <cstring>
#include <assert.h>

#include <QtDebug>


using namespace libfwbuilder;
using namespace fwcompiler;
using namespace std;

string PolicyCompiler_cisco::myPlatformName() { return ""; }

PolicyCompiler_cisco::PolicyCompiler_cisco(FWObjectDatabase *_db,
                                           Firewall *fw,
                                           bool ipv6_policy,
                                           OSConfigurator *_oscnf) :
    PolicyCompiler(_db, fw, ipv6_policy, _oscnf) , helper(this)
{
    // initialize object groups support
    CreateObjectGroups::init(dbcopy);
}

int PolicyCompiler_cisco::prolog()
{
    return PolicyCompiler::prolog();
}

string PolicyCompiler_cisco::createRuleLabel(const string &txt,
                                             Interface *iface,
                                             int rule_num)
{
    ostringstream  str;
    
    str << rule_num;
    if (iface!=NULL) str << "(" << iface->getLabel() << ")";
    else             str << "(" << txt << ")";
    return str.str();
}

void PolicyCompiler_cisco::setAllNetworkZonesToAny()
{
    /* see #1690. After recent changes (aug 2010) in how we deal with
     * network zones, we do not assume that interfaces have network
     * zone "any" if they don't have network zone configured at all.
     * To work around this, will set network zone to "any" on all
     * interfaces. Note that this needs to be done only for IOS ACL and
     * Procurve ACL but not PIX where network zone must be configured
     * by the user.
     */
    list<FWObject*> l2 = fw->getByTypeDeep(Interface::TYPENAME);
    for (list<FWObject*>::iterator i=l2.begin(); i!=l2.end(); ++i)
    {
	Interface *iface = Interface::cast(*i);
        int netzone_id =
            FWObjectDatabase::getIntId(iface->getStr("network_zone"));
        if (netzone_id == -1)
            iface->setStr("network_zone",
                          FWObjectDatabase::getStringId(FWObjectDatabase::ANY_ADDRESS_ID));
    }

}

void PolicyCompiler_cisco::setAllNetworkZonesToNone()
{
    list<FWObject*> l2 = fw->getByTypeDeep(Interface::TYPENAME);
    for (list<FWObject*>::iterator i=l2.begin(); i!=l2.end(); ++i)
    {
	Interface *iface = Interface::cast(*i);
        if (iface->getStr("network_zone") != "")
            iface->setStr("network_zone", "");
    }
}

ciscoACL* PolicyCompiler_cisco::createACLObject(const string &acl_name,
                                                Interface *intf,
                                                const string &dir,
                                                bool using_named_acl)
{
    ciscoACL *acl = new ciscoACL(acl_name, intf, dir, using_named_acl);
    return acl;
}

string PolicyCompiler_cisco::debugPrintRule(Rule *r)
{
    ostringstream str;
    PolicyRule *rule = PolicyRule::cast(r);
    FWObject *rule_iface = dbcopy->findInIndex(rule->getInterfaceId());
    string iname = (rule_iface!=NULL)?rule_iface->getName():"";

    string dir = rule->getDirectionAsString();

    str << PolicyCompiler::debugPrintRule(rule) <<
        " " << dir << " " << iname << " " << rule->getStr("acl") <<
        " intfId=" << rule->getInterfaceId() <<
        " intfstr=" << rule->getInterfaceStr();
    return str.str();
}


void PolicyCompiler_cisco::addDefaultPolicyRule()
{
/*
 * set up backup ssh access to the firewall if told to do so
 */
    if ( getCachedFwOpt()->getBool("mgmt_ssh") &&
         !getCachedFwOpt()->getStr("mgmt_addr").empty() )
    {
        TCPService *ssh = dbcopy->createTCPService();
        ssh->setDstRangeStart(22);
        ssh->setDstRangeEnd(22);
        dbcopy->add(ssh, false);

        TCPService *ssh_rev = dbcopy->createTCPService();
        ssh_rev->setSrcRangeStart(22);
        ssh_rev->setSrcRangeEnd(22);
        dbcopy->add(ssh_rev, false);

        Network *mgmt_workstation = dbcopy->createNetwork();
        mgmt_workstation->setAddressNetmask(
            getCachedFwOpt()->getStr("mgmt_addr"));

        dbcopy->add(mgmt_workstation, false);

        PolicyCompiler::addMgmtRule(
            mgmt_workstation, fw, ssh,
            NULL, PolicyRule::Inbound, PolicyRule::Accept,
            "backup ssh access rule");
    }
}

bool PolicyCompiler_cisco::splitIfSrcAny::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;

    RuleElementSrc *srcrel=rule->getSrc();
    Address *src = compiler->getFirstSrc(rule);

    if ( rule->getDirection()!=PolicyRule::Inbound &&
         ( 
             srcrel->isAny()  ||

             ( srcrel->size()==1 && src!=NULL &&
               !compiler->complexMatch(src,compiler->fw) &&
               srcrel->getBool("single_object_negation")) 
         ) 
    )
    {
         
        PolicyRule *r= compiler->dbcopy->createPolicyRule();
        compiler->temp_ruleset->add(r);
        r->duplicate(rule);
        r->setDirection( PolicyRule::Outbound );

	RuleElementSrc *nsrc=r->getSrc();
        nsrc->clearChildren();
        nsrc->addRef(compiler->fw);

        tmp_queue.push_back(r);
    }
    tmp_queue.push_back(rule);  // add old rule anyway

    return true;
}

bool PolicyCompiler_cisco::splitIfDstAny::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;

    RuleElementSrv *srvrel=rule->getSrv();
    RuleElementDst *dstrel=rule->getDst();
    Address *dst=compiler->getFirstDst(rule);


    std::list<FWObject*> cl;
    for (list<FWObject*>::iterator i1=srvrel->begin(); i1!=srvrel->end(); ++i1) 
    {
        FWObject *o   = *i1;
        FWObject *obj = NULL;
        if (FWReference::cast(o)!=NULL) obj=FWReference::cast(o)->getPointer();
        Service *s=Service::cast(obj);
        assert(s!=NULL);

        if (ICMPService::isA(s))  cl.push_back(s);

        if (TCPService::isA(s) && 
            TCPUDPService::cast(s)->getDstRangeStart()==22 && 
            TCPUDPService::cast(s)->getDstRangeEnd()==22) cl.push_back(s);
        if (TCPService::isA(s) && 
            TCPUDPService::cast(s)->getDstRangeStart()==23 && 
            TCPUDPService::cast(s)->getDstRangeEnd()==23) cl.push_back(s);
    }


    if ( !cl.empty() && rule->getDirection()!=PolicyRule::Outbound &&
         ( 
             dstrel->isAny()  ||

             ( dstrel->size()==1 && dst!=NULL &&
               !compiler->complexMatch(dst,compiler->fw) &&
               dstrel->getBool("single_object_negation")) 
         )
    )
    {
	PolicyRule *r= compiler->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);
	r->setDirection( PolicyRule::Inbound );

	RuleElementDst *ndst=r->getDst();
        ndst->clearChildren();
        ndst->addRef(compiler->fw);

	RuleElementSrv *nsrv=r->getSrv();
        nsrv->clearChildren();
        for (list<FWObject*>::iterator i=cl.begin(); i!=cl.end(); ++i)
            nsrv->addRef( (*i) );

	tmp_queue.push_back(r);
    }
    tmp_queue.push_back(rule); // add old rule in any case

    return true;
}



bool PolicyCompiler_cisco::NegationPhase1::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    return true;

#ifdef DO_NEGATION

    if (compiler->debug>=5) {
	cerr << rule->getLabel() +  "   >>> neg 1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	cerr << rule << " " << compiler->atomicRuleToString( rule );
    }


    RuleElementSrc *src=rule->getSrc();    assert(src);
    RuleElementDst *dst=rule->getDst();    assert(dst);
    RuleElementSrv *srv=rule->getSrv();    assert(srv);


    /* do not use clearChildren because it
     * destroys children objects (can delete
     * rules created on the previous pass)
     */
    compiler->temp_ruleset->clear();
    
    if (src->getNeg()) {

	PolicyRule *r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);
	r->setStr("action","CONTINUE");

	RuleElementSrc *nsrc=r->getSrc();
	nsrc->setNeg(false);
	vr->push_back(r);

	r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);

	nsrc=r->getSrc();
	nsrc->clearChildren();
	nsrc->setAnyElement();
	nsrc->setNeg(false);
	vr->push_back(r);
    }

    if (dst->getNeg())
    {
	PolicyRule *r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);
	r->setStr("action","CONTINUE");

	RuleElementDst *ndst=r->getDst();;
	ndst->setNeg(false);
	vr->push_back(r);

	r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);

	ndst=r->getDst();;
	ndst->clearChildren();
	ndst->setAnyElement();
	ndst->setNeg(false);
	vr->push_back(r);
    }

    if (srv->getNeg())
    {
	PolicyRule *r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);
	r->setStr("action","CONTINUE");

	RuleElementSrv *nsrv=r->getSrv();
	nsrv->setNeg(false);
	vr->push_back(r);

	r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);
	r->duplicate(rule);

	nsrv=r->getSrv();
	nsrv->clearChildren();
	nsrv->setAnyElement();
	nsrv->setNeg(false);
	vr->push_back(r);
    }

    if (vr->empty())
    {
	PolicyRule *r= getCompiler()->dbcopy->createPolicyRule();
	compiler->temp_ruleset->add(r);

	if (compiler->debug>=5) {
	    cerr << "****************** copying rule\n";
	    rule->dump(true,true);
	}
	r->duplicate(rule);
	vr->push_back(r);
    }
    if (compiler->debug>=5) {
	cerr << rule->getLabel() +  "   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
	rule->dump(true,true);
	cerr << " ------------------------------------------------\n";

	for (vector<Rule*>::iterator i=vr->begin(); i!=vr->end(); i++) {
	    Rule *r=(*i);
	    r->dump(true,true);
	    cerr << r << " " << compiler->atomicRuleToString( r );
	}
    }
#endif
}


/**
 *  re_type can be either RuleElementSrc::TYPENAME or RuleElementDst::TYPENAME
 *
 *  TODO: this has to move to class PolicyRuleProcessor
 */
bool PolicyCompiler_cisco::splitIfRuleElementMatchesFW::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    PolicyCompiler_cisco *cisco_comp=dynamic_cast<PolicyCompiler_cisco*>(compiler);

    RuleElement    *re=RuleElement::cast(rule->getFirstByType(re_type));
    int nre=re->size();

    list<FWObject*> cl;

    for (list<FWObject*>::iterator i1=re->begin(); nre>1 && i1!=re->end(); ++i1)
    {
	FWObject *o   = *i1;
	FWObject *obj = NULL;
	if (FWReference::cast(o)!=NULL) obj=FWReference::cast(o)->getPointer();
        Address *a=Address::cast(obj);
        assert(a!=NULL);

//        InetAddr obj_addr=a->getAddress();

        if (cisco_comp->complexMatch(a,cisco_comp->fw)) {

	    cl.push_back(o);   // can not remove right now because remove invalidates iterator
            nre--;

	    PolicyRule  *new_rule= compiler->dbcopy->createPolicyRule();
	    compiler->temp_ruleset->add(new_rule);
	    new_rule->duplicate(rule);
            RuleElement *new_re=RuleElement::cast(new_rule->getFirstByType(re_type));
	    new_re->clearChildren();
	    new_re->setAnyElement();
	    new_re->addRef( a );
	    tmp_queue.push_back(new_rule);
        }
        
    }
    if (!cl.empty())
    {
        for (list<FWObject*>::iterator i1=cl.begin(); i1!=cl.end(); ++i1)
            re->remove( (*i1) );
    }

    tmp_queue.push_back(rule);

    return true;
}

bool PolicyCompiler_cisco::specialCaseWithDynInterface::dropDynamicInterface(
    PolicyRule  *rule, PolicyRule::Direction cmp_dir, RuleElement *re)
{
    PolicyRule::Direction dir=rule->getDirection();
    FWObject *rule_iface = compiler->dbcopy->findInIndex(rule->getInterfaceId());

    list<FWObject*> cl;
    for (list<FWObject*>::iterator i1=re->begin(); i1!=re->end(); ++i1) 
    {
        FWObject *o   = *i1;
        FWObject *obj = o;
        if (FWReference::cast(o)!=NULL) obj=FWReference::cast(o)->getPointer();
        Interface  *ifs   =Interface::cast( obj );

        if (ifs!=NULL && ifs->isDyn()) 
        {
            if (ifs->getId()==rule_iface->getId() &&  dir==cmp_dir)
                cl.push_back(obj);   // keep it
            else
                continue;            // remove it
        } else
            cl.push_back(obj);
    }
    if (re->size()==1 && cl.empty()) // remove the whole rule
        return false;

    if (!cl.empty()) 
    {
        re->clearChildren();
        for (list<FWObject*>::iterator i1=cl.begin(); i1!=cl.end(); ++i1)
        {
            FWObject *oo = *i1;
            re->addRef( oo );
        }
    }
    return true;
}

/**
 * checks for the following situations: 
 *
 *  assuming interface 'INT' is dynamic
 *
 *   src.      rule bound to interface     direction     decision
 * -----------------------------------------------------------------
 *   INT       INT                        outbound      keep
 *   INT       any other                  outbound      remove
 *   INT       INT                        inbound       remove
 *   INT       any other                  inbound       remove
 *            
 *   dest.     rule bound to interface     direction     decision
 * -------------------------------------------------------------------
 *   INT       INT                        inbound       keep
 *   INT       any other                  inbound       remove
 *   INT       INT                        outbound      remove
 *   INT       any other                  outbound      remove
 *
 */
bool PolicyCompiler_cisco::specialCaseWithDynInterface::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;

    if ( dropDynamicInterface( rule, PolicyRule::Outbound, rule->getSrc() ) &&
         dropDynamicInterface( rule, PolicyRule::Inbound,  rule->getDst() ) )
        tmp_queue.push_back(rule);

    return true;
}

/*
 * processor splitIfRuleElementMatchesFW (or one derived from it)
 * should have been called before tcpServiceToFW. This way we know
 * that if dst is a firewall, it is a single object there.
 */
bool PolicyCompiler_cisco::tcpServiceToFW::processNext()
{
    PolicyRule     *rule=getNext(); if (rule==NULL) return false;
    PolicyCompiler_cisco *cisco_comp=dynamic_cast<PolicyCompiler_cisco*>(compiler);
//    RuleElementSrc *src=rule->getSrc();
//    RuleElementDst *dst=rule->getDst();
    RuleElementSrv *srv=rule->getSrv();
    Address  *a=compiler->getFirstDst(rule);
    assert(a!=NULL);

    if (rule->getAction()==PolicyRule::Accept && 
        compiler->complexMatch(a,cisco_comp->fw)) 
    {
        std::list<FWObject*> cl;
        for (list<FWObject*>::iterator i1=srv->begin(); i1!=srv->end(); ++i1) 
        {
            FWObject *o   = *i1;
            FWObject *obj = NULL;
            if (FWReference::cast(o)!=NULL) obj=FWReference::cast(o)->getPointer();
            Service *s=Service::cast(obj);
            assert(s!=NULL);

            if (TCPService::isA(s) && 
                TCPUDPService::cast(s)->getDstRangeStart()==port && 
                TCPUDPService::cast(s)->getDstRangeEnd()==port) cl.push_back(o);
        }
        if (!cl.empty()) 
        {

            PolicyRule  *r= compiler->dbcopy->createPolicyRule();
            compiler->temp_ruleset->add(r);
            r->duplicate(rule);
            RuleElementDst *ndst=r->getDst();
            ndst->clearChildren();
            ndst->setAnyElement();

            // Was commented out in r50
            ndst->addRef( compiler->fw );

            RuleElementSrv *nsrv=r->getSrv();
            nsrv->clearChildren();
            nsrv->add( cl.front() );
            r->setBool("ssh_telnet_cmd",true);
            tmp_queue.push_back(r);

            for (list<FWObject*>::iterator i1=cl.begin(); i1!=cl.end(); ++i1)  
                srv->remove( (*i1) );

            if (srv->size()>0)
                tmp_queue.push_back(rule);
        } else
            tmp_queue.push_back(rule);
    } else
        tmp_queue.push_back(rule);

    return true;
}

/*
 * firewall should be a single object in SRC. If object in SRC matches
 * firewall (in a sence of complexMatch) but is not actual firewall object,
 * do nothing assuming user wanted it that way.
 */
bool PolicyCompiler_cisco::replaceFWinSRCInterfacePolicy::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    FWObject *rule_iface = compiler->dbcopy->findInIndex(rule->getInterfaceId());

    if (rule_iface!=NULL && rule->getDirection()==PolicyRule::Outbound)
    {
        RuleElementSrc *src=rule->getSrc();
        
        if (compiler->getFirstSrc(rule)->getId()==compiler->fw->getId()) 
        {
            src->clearChildren();
            src->addRef(rule_iface);
        }
    }

    tmp_queue.push_back(rule);
    return true;
}

bool PolicyCompiler_cisco::replaceFWinDSTInterfacePolicy::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    FWObject *rule_iface = compiler->dbcopy->findInIndex(rule->getInterfaceId());

    if (rule_iface!=NULL && rule->getDirection()==PolicyRule::Inbound)
    {
        RuleElementDst *dst=rule->getDst();

        if (compiler->getFirstDst(rule)->getId()==compiler->fw->getId()) 
        {
            dst->clearChildren();
            dst->addRef(rule_iface);
        }
    }

    tmp_queue.push_back(rule);
    return true;
}

/*
 * dst should contain objects that belong to the network zone of the
 * same interface (use splitByNetworkZonesForRE for that)
 */
bool PolicyCompiler_cisco::replaceFWinDSTPolicy::processNext()
{
    Helper helper(compiler);
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    FWObject *rule_iface = compiler->dbcopy->findInIndex(rule->getInterfaceId());

    if (rule_iface==NULL)
    {
        RuleElementSrc *src=rule->getSrc();
        RuleElementDst *dst=rule->getDst();

        if (!src->isAny() && compiler->getFirstDst(rule)->getId()==compiler->fw->getId()) 
        {
            try
            {
                int iface_id = helper.findInterfaceByNetzone(
                    compiler->getFirstSrc(rule));
                FWObject *iface = compiler->dbcopy->findInIndex(iface_id);

                dst->clearChildren();
                dst->addRef(iface);
            } catch (string addr)
            {
                ostringstream str;
                str << "Address " << addr
                    << " does not match address or network zone of any interface." ;
                compiler->abort(rule, str.str());
                return true;
            }
        }
    }

    tmp_queue.push_back(rule);
    return true;
}

void PolicyCompiler_cisco::splitByNetworkZonesForRE::AddToInterface(
    int interface_id, Address *addr, PolicyRule *rule)
{
    PolicyRule  *new_rule;
    RuleElement *new_re;

    new_rule = rules[interface_id];
    if (new_rule==NULL) 
    {
        new_rule = compiler->dbcopy->createPolicyRule();
        compiler->temp_ruleset->add(new_rule);
        new_rule->duplicate(rule);
        rules[interface_id]=new_rule;
        new_re=RuleElement::cast(new_rule->getFirstByType(re_type));
        new_re->clearChildren();
        new_re->setAnyElement();
    }
    new_re=RuleElement::cast(new_rule->getFirstByType(re_type));
    new_re->addRef( addr );
}

bool PolicyCompiler_cisco::splitByNetworkZonesForRE::processNext()
{
    Helper helper(compiler);
    PolicyRule *rule=getNext(); if (rule==NULL) return false;
    RuleElement    *re=RuleElement::cast(rule->getFirstByType(re_type));
    if (re->size()==1) 
    {
        tmp_queue.push_back(rule);
        return true;
    }
    
    rules.clear();

    std::list<FWObject*> cl;
    for (list<FWObject*>::iterator i1=re->begin(); i1!=re->end(); ++i1) 
    {
	FWObject *o   = *i1;
	FWObject *obj = NULL;
	if (FWReference::cast(o)!=NULL) obj=FWReference::cast(o)->getPointer();
        Address *a=Address::cast(obj);
        assert(a!=NULL);

//        InetAddr obj_addr=a->getAddress();

        try
        {
            int interface_id = helper.findInterfaceByNetzone(a);
            AddToInterface(interface_id, a, rule);
        } catch (string err)
        {
            // could not find interface with netzone to match address 'a'
            // will assign rule to all interfaces. Act as if all interfaces
            // had network zone 'any' and each matches this address.

            // issue warning only if platform uses netwrk zones.

            bool supports_network_zones =
                Resources::getTargetCapabilityBool(
                    compiler->fw->getStr("platform"), "network_zones");

            if (supports_network_zones)
                compiler->warning(rule, err);

            FWObjectTypedChildIterator i =
                compiler->fw->findByType(Interface::TYPENAME);
            for ( ; i!=i.end(); ++i)
            {
                Interface  *ifs = Interface::cast(*i);
                AddToInterface(ifs->getId(), a, rule);
            }
        }
    }
    for (std::map<int,PolicyRule*>::iterator i=rules.begin();
         i!=rules.end(); ++i)
    {
        tmp_queue.push_back((*i).second);        
    }
    return true;
}

bool PolicyCompiler_cisco::equalObjCISCO::operator()(FWObject *o)
{
    if (ICMPService::cast(obj)!=NULL && ICMPService::cast(o)!=NULL)
    {
        return (obj->getInt("type")==o->getInt("type"));
    } else
        return o->getId()==obj->getId();
}

/*  re_type can be either RuleElementSrc::TYPENAME or RuleElementDst::TYPENAME */
bool PolicyCompiler_cisco::removeRedundantAddresses::processNext()
{
    PolicyRule *rule=getNext(); if (rule==NULL) return false;

    RuleElement *re=RuleElement::cast(rule->getFirstByType(re_type));
    if (re->size()==1) 
    {
        tmp_queue.push_back(rule);
        return true;
    }

    map<Address*, bool> status;
    for (list<FWObject*>::iterator i1=re->begin(); i1!=re->end(); ++i1) 
    {
        Address *a = Address::cast(FWReference::getObject(*i1));
        assert(a!=NULL);   // assuming all objects are addresses.
        status[a] = false;
    }

    map<Address*,bool>::iterator i1;
    map<Address*,bool>::iterator i2;
    for (i1=status.begin(); i1!=status.end(); ++i1)
    {
        Address *a1 = i1->first;
        // const InetAddrMask* am1 = a1->getInetAddrMaskObjectPtr();

        for (i2=status.begin(); i2!=status.end(); ++i2)
        {
            if (i2->second) continue;

            Address *a2 = i2->first;
            if (a1->getId() == a2->getId()) continue;

            // const InetAddrMask* am2 = a2->getInetAddrMaskObjectPtr();
            // if (am1 && am2 && am1->toString() == am2->toString()) continue;

            if (compiler->checkForShadowing(*a1, *a2) ) status[a1] = true;
        }
    }

    for (i1=status.begin(); i1!=status.end(); ++i1)
    {
        if (i1->second) re->removeRef(i1->first);
    }

    tmp_queue.push_back(rule);
    return true;
}

bool PolicyCompiler_cisco::processMultiAddressObjectsInRE::processNext()
{
    PolicyRule *rule = getNext(); if (rule==NULL) return false;
    RuleElement *re = RuleElement::cast( rule->getFirstByType(re_type) );

    for (FWObject::iterator i=re->begin(); i!=re->end(); i++)
    {
        FWObject *o = *i;
        if (FWReference::cast(o)!=NULL) o = FWReference::cast(o)->getPointer();
        MultiAddress *atrt = MultiAddress::cast(o);
        if (atrt!=NULL && atrt->isRunTime())
        {
            compiler->abort(
                    rule, 
                    "Run-time AddressTable and DNSName objects are not supported.");
            return true;
        }
    }

    tmp_queue.push_back(rule);
    return true;
}

void PolicyCompiler_cisco::compile()
{
}

class acl_sort_order
{
    public:
    acl_sort_order() {};
    bool operator()(const string &a, const string &b)
    {
        string::size_type i1,i2;
        i1=a.find(' ',a.find(' ')+1);
        i2=b.find(' ',b.find(' ')+1);
        return a.substr(0,i1) < b.substr(0,i2);
    }
};

void PolicyCompiler_cisco::regroup()
{
    list<string>              commands;
    map<string,list<string> > script;

    commands.push_back("THE_REST");
    commands.push_back("access-list ");
    commands.push_back("access-group ");
    commands.push_back("icmp ");
    commands.push_back("ssh ");
    commands.push_back("telnet ");

    string acl, agrp, icmp, telnet, ssh;
    string new_output;
    char   buf[1024];

    istringstream in(output.str());

    while (in)
    {
        in.getline(buf, 1023, '\n');

        strcat(buf,"\n");

        if (buf[0]=='!') continue;

        string slot="THE_REST";
        string cmd(buf);
        string::size_type n=cmd.find(' ');
        
        list<string>::iterator s = ::find(commands.begin(),commands.end(),cmd.substr(0,n+1));
        if (s!=commands.end()) slot = *s;

        script[slot].push_back(buf);
    }

    script["access-list "].sort(acl_sort_order());

    output.str("");

    for (list<string>::iterator i=commands.begin(); i!=commands.end(); ++i)
    {
        for (list<string>::iterator j=script[*i].begin(); j!=script[*i].end(); ++j)
            output << *j;
        output << "! \n";
        output << "! \n";
    }
}

void PolicyCompiler_cisco::epilog()
{
}

