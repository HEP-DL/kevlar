/**
  TODO:: Stop writing documentation before v0.5
**/
#ifndef HDF5Image_hh
#define HDF5Image_hh

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <string>

namespace fhicl{
  class ParameterSet;
}
namespace art{
  class Event;
  class SubRun;  
}

namespace kevlar{
  class HDF5Image : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fOutFileName;
    std::string fDataSetName;
    //H5::H5File fH5Output;
  public:
    HDF5Image(::fhicl::ParameterSet const& );
    ~HDF5Image();
    void analyze(::art::Event const&) override;
    void beginSubRun(::art::SubRun const&) override;
    void endSubRun(::art::SubRun const&) override;
  };
}

DEFINE_ART_MODULE(kevlar::HDF5Image)
#endif //HDF5Image_hh
