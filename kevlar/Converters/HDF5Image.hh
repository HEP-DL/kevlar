/**
  TODO:: Stop writing documentation before v0.5
**/
#ifndef HDF5Image_hh
#define HDF5Image_hh

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <boost/multi_array.hpp>
#include "H5Cpp.h"
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
    std::string fDataSetName;
    hsize_t fDims[4];// Dataset dimensions
    hsize_t fMaxDims[4];
    hsize_t fChunkDims[4];// Chunk Dimensions
    H5::DataSpace fDataSpace;/// Buffer
    H5::DSetCreatPropList fParms;// IO Parameters
    H5::DataSet* fDataSet;// points at dataset
    int fFillValue;
    uint32_t fNEvents;
    boost::multi_array<int, 4>  fBuffer;
    uint32_t fBufferCounter;

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
