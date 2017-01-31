/**
  TODO:: Stop writing documentation before v0.5
**/
#ifndef HDF5Image
#define HDF5Image

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"

class fhicl::ParameterSet;
class art::Event;
class art::SubRun;
class std::string;
class std::ofstream;

namespace kevlar{
  class HDF5Image : public art::EDAnalyzer {
    std::string fProducerName;
    std::string fOutFileName;
    H5::H5File fH5Output;
  public:
    HDF5Image(fhicl::ParameterSet const & p);
    ~HDF5Image();
    void analyze(art::Event const & e) override;
    void beginSubRun(art::SubRun const & sr) override;
    void endSubRun(art::SubRun const & sr) override;
  };
}

DEFINE_ART_MODULE(kevlar::HDF5Image)
#endif //HDF5Image