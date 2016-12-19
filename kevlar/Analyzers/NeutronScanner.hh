/**
  No inline documentation for you
**/
#ifndef NeutronScanner_hh_
#define NeutronScanner_hh_

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include <fstream>
#include <string>

namespace kevlar{
  class NeutronScanner : art::EDAnalyzer {
    std::string fOutFileName;
    std::ofstream fCSVOut;
  public:
    NeutronScanner(fhicl::ParameterSet const & p);
    ~NeutronScanner();
    void analyze(art::Event const & e) override;
    void beginSubRun(art::SubRun const & sr) override;
    void endSubRun(art::SubRun const & sr) override;
  };
}

DEFINE_ART_MODULE(kevlar::NeutronScanner)
#endif //NeutronScanner_hh_