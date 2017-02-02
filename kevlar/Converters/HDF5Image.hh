/**
  TODO:: Stop writing documentation before v0.5
**/
#ifndef HDF5Image_hh
#define HDF5Image_hh

#define HDF5Image_RANK 3

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <string>

#include "H5Cpp.h"

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
    std::string fDataSetName;
    hsize_t fDims[HDF5Image_RANK];
    H5::DataSpace fDataSpace;
    H5::DataSet* fDataSet;
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
