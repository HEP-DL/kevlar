#ifndef HDF5ParticleLabelSS_HH_
#define HDF5ParticleLabelSS_HH_

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

  class HDF5ParticleLabelSS : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fG4Name;
    std::string fDataSetName;
    std::vector<std::string> fLabels ;
    hsize_t fDims[2];// Dataset dimensions
    hsize_t fMaxDims[2];// Maximum Data Dimensions
    hsize_t fChunkDims[2];// Chunk Dimensions
    H5::DataSpace fDataSpace;/// Buffer
    H5::DSetCreatPropList fParms;// IO Parameters
    H5::DataSet* fDataSet;// points at dataset
    int fFillValue;
    uint32_t fNEvents;// dimension in event space
    uint32_t fNInstances;// dimension in instance space

  public:
    HDF5ParticleLabelSS(::fhicl::ParameterSet const& );
    ~HDF5ParticleLabelSS();
    void analyze(::art::Event const&) override;
    void beginJob() override;
  };

}

DEFINE_ART_MODULE(kevlar::HDF5ParticleLabelSS)
#endif // HDF5ParticleLabelSS_HH_
