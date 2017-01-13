/**
  No inline documentation for you
**/
#ifndef NeutronPhotonYield_hh_
#define NeutronPhotonYield_hh_

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <string>

namespace kevlar{


  class NeutronPhotonYield : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fOutFileName;
    std::ofstream fCSVOut;

  public:
    NeutronPhotonYield(fhicl::ParameterSet const & p);
    ~NeutronPhotonYield();
    void analyze(art::Event const & e) override;
    void beginSubRun(art::SubRun const & sr) override;
    void endSubRun(art::SubRun const & sr) override;
  };
}

DEFINE_ART_MODULE(kevlar::NeutronPhotonYield)
#endif //NeutronPhotonYield_hh_