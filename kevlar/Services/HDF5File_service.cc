#include "kevlar/Services/HDF5File.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

#include <iostream>

namespace kevlar{

  HDF5File::HDF5File(fhicl::ParameterSet const& pset, art::ActivityRegistry&):
    fOutput(pset.get<std::string>("FileName","output.h5"),H5F_ACC_TRUNC ),
    fDataSets(),
    fGroups()
  {

  }

  HDF5File::~HDF5File()
  {
    for (auto it: this->fDataSets){
      delete it;
    }
    for (auto it: this->fGroups){
      delete it;
    }
  }

  H5::Group HDF5File::GetGroup(std::string& name){
    try{
      return this->fOutput.createGroup(name);
    }
    catch(const std::exception& e){
      return this->fOutput.getGroup(name);
    }
    return this->fOutput.getGroup(name);
  }

  H5::DataSet* HDF5File::CreateDataSet(std::string& name,  std::string& group_name,
    H5::DataSpace& space,
    H5::DSetCreatPropList& plist)
  {
    H5::Group group = this->GetGroup(group_name);

    H5::DataSet* data = new H5::DataSet(group.createDataSet(name.c_str(), 
                                H5::PredType::NATIVE_INT,
                                space, plist) );
    this->fDataSets.push_back(data);
    return data;
  }
}

DEFINE_ART_SERVICE(kevlar::HDF5File)
