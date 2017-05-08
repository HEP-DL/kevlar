#include "kevlar/Converters/HDF5ParticleLabelVector.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "TDatabasePDG.h"

#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>


namespace kevlar{

  class PDGNameNotFound: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "Name in particle label vector is not in PDG DB";
    }
  };

  HDF5ParticleLabelVector::HDF5ParticleLabelVector(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      HDF5Mixin<2, HDF5ParticleLabelVector>(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels"))
  {
  }

  HDF5ParticleLabelVector::~HDF5ParticleLabelVector()
  {

  }

  void HDF5ParticleLabelVector::analyze(art::Event const & evt)
  {
    
    art::Handle< std::vector< simb::MCTruth > > mct_handle;
    std::cout<<"HDF5ParticleLabelVector:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle){
      for(int i=0; i<truth.NParticles(); ++i){
        int pdg = truth.GetParticle(i).PdgCode();
        if (abs(pdg) > 1E7) continue; // don't try GetName() on the Argon nucleus, e.g.
        std::cout<<"Found particle: "<<pdg<<" "<<truth.GetParticle(i).Process()<<std::endl;
        auto particle = TDatabasePDG::Instance()->GetParticle(pdg);
        if(!particle)
          continue;
        std::string name = particle->GetName();

        ptrdiff_t index = std::find(fLabels.begin(), fLabels.end(), name) - fLabels.begin();

        if(index < int(fLabels.size()) && index>=0 ){
          std::cout<<"Found Particle with name: "<<name<< " and output vector index: "<<index<<std::endl;


          fBuffer[fBufferCounter][index] = fBuffer[fBufferCounter][index] + 1;
        }
        else{
          std::cout<<"Found Particle Outside Label Table: "<<name<<std::endl;       
        }
      }
    }
    this->atEvent();
  }
  
  void HDF5ParticleLabelVector::beginJob()
  {
    kevlar::HDF5File* _OutputFile = this->atDataSetSetup();

     H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
     H5::StrType strdatatype(H5::PredType::C_S1, 256);

     {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index0");
      const std::string strwritebuf ("particle_type");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    uint32_t label_index=0;
    for (auto label : fLabels){

      // Set up write buffer for attribute
      std::stringstream name;
      name<<"label_index"<<label_index;

      const std::string ATTR_NAME (name.str());
      const std::string strwritebuf (label);

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf);

      label_index+=1;

    }
  }

  void HDF5ParticleLabelVector::endJob()
  {

  }
}
