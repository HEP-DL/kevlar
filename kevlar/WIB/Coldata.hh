#ifndef Coldata_hh_
#define Coldata_hh_

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <boost/multi_array.hpp>
#include <string>
#include <vector>


namespace fhicl{
  class ParameterSet;
}


namespace art{
  class Event;
  class SubRun;  
}


namespace kevlar{

  class Coldata : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fG4Name;
    std::vector<std::string> fLabels ;
    uint32_t fNEvents;
    std::string fTime;
    std::chrono::nanoseconds fNanoTime;


  public:
    Coldata(::fhicl::ParameterSet const& );
    ~Coldata();
    void analyze(::art::Event const&) override;
    void beginJob() override;
    void endJob() override;
  };

}

DEFINE_ART_MODULE(kevlar::Coldata)
#endif // Coldata_hh_
