#ifndef HDF5FILE_HH
#define HDF5FILE_HH

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "H5Cpp.h"
#include <string>
#include <vector>

namespace H5{
  class H5File;
  class Group;
  class DataSet;
  class DataSpace;
  class DSetCreatePropList;
}

namespace art{
  class ActivityRegistry;
}

namespace fhicl{
  class ParameterSet;
}

namespace kevlar{
  class HDF5File {
    H5::H5File fOutput;
    std::vector<H5::DataSet*> fDataSets;
    std::vector<H5::Group*> fGroups;
  public:
    HDF5File(fhicl::ParameterSet const&, art::ActivityRegistry&);
    ~HDF5File();
    H5::DataSet* CreateDataSet(std::string& name,std::string& group, H5::DataSpace& space,
      H5::DSetCreatPropList& plist);
  protected:
    H5::Group GetGroup(std::string&);
  };
}

DECLARE_ART_SERVICE(kevlar::HDF5File, LEGACY)

#endif // HDF5FILE_HH
