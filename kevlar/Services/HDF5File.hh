#ifndef HDF5FILE_HH
#define HDF5FILE_HH

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "H5Cpp.h"
#include <string>
#include <vector>

namespace H5{
  class H5File;
  class DataSet;
  class DataSpace;
  class DSetCreatePropList;
}

namespace kevlar{
  class HDF5File {
    H5::H5File fOutput;
    std::vector<H5::DataSet*> fDataSets;
  public:
    HDF5File(fhicl::ParameterSet const&);
    ~HDF5File();
    H5::DataSet* CreateDataSet(std::string& name, H5::DataSpace& space,
      H5::DSetCreatPropList& plist);
  };
}

DECLARE_ART_SERVICE_INTERFACE(kevlar::HDF5File, LEGACY)

#endif // HDF5FILE_HH
