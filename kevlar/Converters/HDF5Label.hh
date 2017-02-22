#ifndef HDF5LABEL_HH
#define HDF5LABEL_HH

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <boost/multi_array.hpp>
#include "H5Cpp.h"
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

  class HDF5Label : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fDataSetName;
    std::vector<std::string> fLabels;
    hsize_t fDims[2];// Dataset dimensions
    hsize_t fMaxDims[2];// Maximum Data Dimsions
    hsize_t fChunkDims[2];// Chunk Dimensions
    H5::DataSpace fDataSpace;/// Buffer
    H5::DSetCreatPropList fParms;// IO Parameters
    H5::DataSet* fDataSet;// points at dataset
    int fFillValue;
    uint32_t fNEvents;
    boost::multi_array<int, 2>  fBuffer;
    uint32_t fBufferCounter;
  public:
    HDF5Label(::fhicl::ParameterSet const& );
    ~HDF5Label();
    void analyze(::art::Event const&) override;
    void beginSubRun(::art::SubRun const&) override;
    void endSubRun(::art::SubRun const&) override;
  };

}

DEFINE_ART_MODULE(kevlar::HDF5Label)
#endif // HDF5LABEL_HH
