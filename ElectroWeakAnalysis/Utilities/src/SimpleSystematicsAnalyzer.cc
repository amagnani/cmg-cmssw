////////// Header section /////////////////////////////////////////////
#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/ParameterSet/interface/InputTag.h"

class SimpleSystematicsAnalyzer: public edm::EDFilter {
public:
      SimpleSystematicsAnalyzer(const edm::ParameterSet& pset);
      virtual ~SimpleSystematicsAnalyzer();
      virtual bool filter(edm::Event &, const edm::EventSetup&);
      virtual void beginJob(const edm::EventSetup& eventSetup) ;
      virtual void endJob() ;
private:
      std::string selectorPath_;
      std::vector<edm::InputTag> weightTags_;
      unsigned int originalEvents_;
      std::vector<double> weightedEvents_;
      unsigned int selectedEvents_;
      std::vector<double> weightedSelectedEvents_;
};

////////// Source code ////////////////////////////////////////////////
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/Framework/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"

/////////////////////////////////////////////////////////////////////////////////////
SimpleSystematicsAnalyzer::SimpleSystematicsAnalyzer(const edm::ParameterSet& pset) :
  selectorPath_(pset.getUntrackedParameter<std::string> ("SelectorPath","")),
  weightTags_(pset.getUntrackedParameter<std::vector<edm::InputTag> > ("WeightTags")) { 
}

/////////////////////////////////////////////////////////////////////////////////////
SimpleSystematicsAnalyzer::~SimpleSystematicsAnalyzer(){}

/////////////////////////////////////////////////////////////////////////////////////
void SimpleSystematicsAnalyzer::beginJob(const edm::EventSetup& eventSetup){
      originalEvents_ = 0;
      selectedEvents_ = 0;
      edm::LogVerbatim("SimpleSystematicsAnalysis") << "Uncertainties will be determined for the following tags: ";
      for (unsigned int i=0; i<weightTags_.size(); ++i) {
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "\t" << weightTags_[i].encode();
            weightedEvents_.push_back(0.);
            weightedSelectedEvents_.push_back(0.);
      }
}

/////////////////////////////////////////////////////////////////////////////////////
void SimpleSystematicsAnalyzer::endJob(){
      if (originalEvents_==0) {
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "NO EVENTS => NO RESULTS";
            return;
      }
      if (selectedEvents_==0) {
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "NO SELECTED EVENTS => NO RESULTS";
            return;
      }

      edm::LogVerbatim("SimpleSystematicsAnalysis") << "\n>>>> Begin of Weight systematics summary >>>>";
      edm::LogVerbatim("SimpleSystematicsAnalysis") << "Total number of analyzed data: " << originalEvents_ << " [events]";
      double originalAcceptance = double(selectedEvents_)/originalEvents_;
      edm::LogVerbatim("SimpleSystematicsAnalysis") << "Total number of selected data: " << selectedEvents_ << " [events], corresponding to acceptance: " << originalAcceptance*100 << " [%]";
      
      for (unsigned int i=0; i<weightTags_.size(); ++i) {
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "Results for Weight Tag: " << weightTags_[i].encode() << " ---->";

            double acc_central = 0.;
            if (weightedEvents_[i]>0) acc_central = weightedSelectedEvents_[i]/weightedEvents_[i]; 
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "\tTotal Events after reweighting: " << weightedEvents_[i] << " [events]";
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "\tEvents selected after reweighting: " << weightedSelectedEvents_[i] << " [events]";
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "\tAcceptance after reweighting: " << acc_central*100 << " [%]";
            edm::LogVerbatim("SimpleSystematicsAnalysis") << "\ti.e. " << std::setprecision(4) << 100*(acc_central/originalAcceptance-1.) << "% variation with respect to the original acceptance";

      }
      edm::LogVerbatim("SimpleSystematicsAnalysis") << ">>>> End of Weight systematics summary >>>>";

}

/////////////////////////////////////////////////////////////////////////////////////
bool SimpleSystematicsAnalyzer::filter(edm::Event & ev, const edm::EventSetup&){
      originalEvents_++;

      bool selectedEvent = false;
      edm::Handle<edm::TriggerResults> triggerResults;
      if (!ev.getByLabel(edm::InputTag("TriggerResults"), triggerResults)) {
            edm::LogError("SimpleSystematicsAnalysis") << ">>> TRIGGER collection does not exist !!!";
            return false;
      }

      edm::TriggerNames trigNames;
      trigNames.init(*triggerResults);
      unsigned int pathIndex = trigNames.triggerIndex(selectorPath_);
      bool pathFound = (pathIndex>=0 && pathIndex<trigNames.size());
      if (pathFound) {
            if (triggerResults->accept(pathIndex)) selectedEvent = true;
      }
      //edm::LogVerbatim("SimpleSystematicsAnalysis") << ">>>> Path Name: " << selectorPath_ << ", selected? " << selectedEvent;

      if (selectedEvent) selectedEvents_++;

      for (unsigned int i=0; i<weightTags_.size(); ++i) {
            edm::Handle<double> weightHandle;
            ev.getByLabel(weightTags_[i], weightHandle);
            weightedEvents_[i] += (*weightHandle);
            if (selectedEvent) weightedSelectedEvents_[i] += (*weightHandle);
      }

      return true;
}

DEFINE_FWK_MODULE(SimpleSystematicsAnalyzer);
