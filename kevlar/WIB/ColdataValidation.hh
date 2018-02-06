#ifndef ColdataValidation_hh_
#define ColdataValidation_hh_

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <boost/multi_array.hpp>
#include <string>
#include <vector>
#include <chrono>

namespace fhicl{
  class ParameterSet;
}


namespace art{
  class Event;
  class SubRun;  
}


namespace kevlar{

  class ColdataValidation : public art::EDProducer {

    uint32_t fNEvents;
    std::string fTime;
    std::chrono::nanoseconds fNanoTime;
    std::string fFilePath;


  public:
    ColdataValidation(::fhicl::ParameterSet const& );
    ~ColdataValidation();
    void produce(::art::Event &) override;
    void beginJob() override;
    void endJob() override;
  };

}

DEFINE_ART_MODULE(kevlar::ColdataValidation)
#endif // ColdataValidation_hh_
