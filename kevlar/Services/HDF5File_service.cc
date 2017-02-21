#include "kevlar/Services/HDF5File.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

namespace kevlar{

  HDF5File::HDF5File(fhicl::ParameterSet const& pset, art::ActivityRegistry&):
    fOutput(pset.get<std::string>("FileName","output.h5"),H5F_ACC_TRUNC | H5F_ACC_DEBUG | H5F_ACC_RDWR),
    fDataSets()
  {

  }
  HDF5File::~HDF5File()
  {
    for (auto it: this->fDataSets){
      delete it;
    }
  }
  H5::DataSet* HDF5File::CreateDataSet(std::string& name, 
    H5::DataSpace& space,
    H5::DSetCreatPropList& plist)
  {
    H5::DataSet* data = new H5::DataSet(this->fOutput.createDataSet(name.c_str(), 
                                H5::PredType::NATIVE_INT,
                                space, plist) );
    this->fDataSets.push_back(data);
    return data;
  }
}
DEFINE_ART_SERVICE(kevlar::HDF5File)