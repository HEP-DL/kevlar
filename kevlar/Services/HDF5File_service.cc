#include "kevlar/Services/HDF5File.hh"
#include "kevlar/Utilites/Version.hh"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

#include <iostream>

namespace kevlar{

  HDF5File::HDF5File(fhicl::ParameterSet const& pset, art::ActivityRegistry&):
    fOutput(pset.get<std::string>("FileName","output.h5"),H5F_ACC_TRUNC ),
    fDataSets()
  {
    // Setting up H5 to not print exceptions allows the group creation
    // Pattern to function without vomiting all over the terminal output
    if(! pset.get<bool>("PrintErrors", true) )
      H5::Exception::dontPrint();

    H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("kevlar_version");
      const std::string strwritebuf (KEVLAR_VERSION);

      // Create attribute and write to it
      H5::Attribute myatt_in = fOutput.createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }
  }

  HDF5File::~HDF5File()
  {
    for (auto it: this->fDataSets){
      delete it;
    }
  }

  H5::Group HDF5File::GetGroup(std::string& name){
    try{
      return this->fOutput.openGroup(name);
    }
    catch(const H5::Exception& e){
      return this->fOutput.createGroup(name);
    }
    return this->fOutput.createGroup(name);
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
