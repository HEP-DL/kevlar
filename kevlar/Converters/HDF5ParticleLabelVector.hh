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


  class HDF5ParticleLabelVector : public art::EDAnalyzer, public HDF5Mixin<2, HDF5ParticleLabelVector> {
    std::string fProducerName;
    std::string fDataSetName;
    std::vector<std::string> fLabels;
  protected:
    struct HDF5DataSetProperties
    {
      static constexpr const std::string group = "label";
    };

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
