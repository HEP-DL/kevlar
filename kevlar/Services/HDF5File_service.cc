#include "kevlar/Services/HDF5File.hh"

#include <stringstream>


namespace kevlar{

  HDF5File::HDF5File(fhicl::ParameterSet const& pset):
    fOutput(pset.get<std::string>("FileName","output.h5"),H5F_ACC_TRUNC),
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
    H5::DataSet* data = new H5::DataSet(this->fOutput->createDataSet(name, 
                                H5::PredType::NATIVE_INT,
                                space) );
    this->fDataSets.push_back(data);
    return data;
  }
}
