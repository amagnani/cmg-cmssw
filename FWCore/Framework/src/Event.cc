/*----------------------------------------------------------------------
$Id: Event.cc,v 1.19 2005/09/01 23:56:11 wmtan Exp $
----------------------------------------------------------------------*/

#include <memory>
#include <vector>
#include <algorithm>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/EDProduct/interface/ProductID.h"
#include "FWCore/Framework/interface/EventPrincipal.h"
#include "FWCore/Framework/interface/ProductDescription.h"
#include "FWCore/Framework/src/Group.h"

using namespace std;

namespace edm {

  Event::Event(EventPrincipal& ep, const ModuleDescription& md) :
    ep_(ep),
    md_(md)
  {  }

  struct deleter {
    void operator()(const std::pair<EDProduct*, std::string> p) const { delete p.first; }
  };

  Event::~Event() {
    // anything left here must be the result of a failure
    // let's record them as failed attempts in the event principal
    std::for_each(put_products_.begin(),put_products_.end(),deleter());
  }

  EventID
  Event::id() const
  {
    return ep_.id();
  }

  Timestamp
  Event::time() const
  {
    return ep_.time();
  }

  void 
  Event::commit_() {
    // fill in guts of provenance here
    ProductPtrVec::iterator pit(put_products_.begin());
    ProductPtrVec::iterator pie(put_products_.end());

    while(pit!=pie) {
	auto_ptr<EDProduct> pr(pit->first);
	// note: ownership has been passed - so clear the pointer!
	pit->first = 0;
	ProductDescription desc(md_,
			TypeID(*pr).userClassName(),
			TypeID(*pr).friendlyClassName(),
			pit->second,
			0);

	auto_ptr<Provenance> pv(new Provenance(desc));

	// set parts of provenance
	pv->event.cid = 0; // TODO: what is this supposed to be?
	pv->event.status = BranchEntryDescription::Success;
	pv->event.parents = gotProductIDs_;

	ep_.put(pr,pv);
	++pit;
    }

    // the cleanup is all or none
    put_products_.clear();
  }

  BasicHandle
  Event::get_(ProductID const& oid) const
  {
    return ep_.get(oid);
  }

  BasicHandle
  Event::get_(TypeID const& id, Selector const& sel) const
  {
    return ep_.getBySelector(id, sel);
  }
    
  BasicHandle
  Event::getByLabel_(TypeID const& id,
		     const string& label,
                     const string& productInstanceName) const
  {
    return ep_.getByLabel(id, label, productInstanceName);
  }

  void 
  Event::getMany_(TypeID const& id, 
		  Selector const& sel,
		  BasicHandleVec& results) const
  {
    ep_.getMany(id, sel, results);
  }

  BasicHandle
  Event::getByType_(TypeID const& id) const
  {
    return ep_.getByType(id);
  }

  void 
  Event::getManyByType_(TypeID const& id, 
		  BasicHandleVec& results) const
  {
    ep_.getManyByType(id, results);
  }

  Provenance const&
  Event::getProvenance(ProductID const& oid) const
  {
    return ep_.getProvenance(oid);
  }

  void
  Event::getAllProvenance(std::vector<Provenance const*> & provenances) const
  {
    ep_.getAllProvenance(provenances);
  }
}
