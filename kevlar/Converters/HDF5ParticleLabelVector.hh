#ifndef HDF5ParticleLabelVector_HH
#define HDF5ParticleLabelVector_HH

#include "kevlar/Framework/HDF5Mixin.hh"
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


  struct HDF5ParticleLabelVectorProperties{
    std::string group;
  };

  class HDF5ParticleLabelVector : public art::EDAnalyzer {
    std::vector<std::string> fLabels;
    hsize_t fDims[2];
    hsize_t fMaxDims[2];
    hsize_t fChunkDims[2];
    std::string fProducerName;
    std::string fDataSetName;
    H5::DataSpace fDataSpace;/// Buffer
    H5::DSetCreatPropList fParms;// IO Parameters
    H5::DataSet* fDataSet;// points at dataset
    int fFillValue;
    uint32_t fNEvents;
    boost::multi_array<int, 2>  fBuffer;
    uint32_t fBufferCounter;
  public:
    HDF5ParticleLabelVector(::fhicl::ParameterSet const& );
    ~HDF5ParticleLabelVector();
    void analyze(::art::Event const&) override;
    void beginJob() override;
    void endJob() override;
  };

}

DEFINE_ART_MODULE(kevlar::HDF5ParticleLabelVector)
#endif // HDF5ParticleLabelVector_HH