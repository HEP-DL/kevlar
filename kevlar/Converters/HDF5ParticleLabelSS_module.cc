#include "kevlar/Converters/HDF5ParticleLabelSS.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "TDatabasePDG.h"

#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>


namespace kevlar{


  HDF5ParticleLabelSS::HDF5ParticleLabelSS(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","generator")),
      fG4Name(pSet.get<std::string>("G4Label","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels")),
      fDims{
        0, 
        6,
      },
      //Index: [event, instance, particle, plane, wire, time]
      fMaxDims{
        H5S_UNLIMITED,
        6
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSizeN",1),
        6
      },
      fDataSpace(2, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0),
      fNInstances(0)
  {
      fParms.setChunk( 2, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",7));
  }

  HDF5ParticleLabelSS::~HDF5ParticleLabelSS()
  {

  }

  void HDF5ParticleLabelSS::analyze(art::Event const & evt)
  {
    auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double vd = detprop->DriftVelocity(); //cm/musec
    art::ServiceHandle<geo::Geometry> geo;

    (this->fNEvents)++;


    art::Handle< std::vector< simb::MCParticle > > mct_handle;
    std::cout<<"HDF5ParticleLabelSS:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle)
    {

      int pdg = truth.PdgCode();
      auto particle = TDatabasePDG::Instance()->GetParticle(pdg);
      if(!particle)  continue;
      std::string name = particle->GetName();
      ptrdiff_t index = std::find(fLabels.begin(), fLabels.end(), name) - fLabels.begin();

      if(index < int(fLabels.size()) && index>=0 )
      {
        std::cout<<"Found Particle with name: "<<name<< " and output vector index: "<<index<<std::endl;
        int tmp_instances=0;

        for (auto traj = truth.Trajectory().begin() ; traj != truth.Trajectory().end(); ++traj )
        {
          const TVector3 position = traj->first.Vect();
          bool foundWires=true;
          int wire[3]={0,0,0};
          for(size_t plane=0; plane < 3; plane++)
          {
            try
            {
              wire[plane] = geo->NearestWire( position, plane);
            }
            catch(cet::exception& e )
            {
              std::cout<<"Could NOT find nearest Wire to: "<<position[0]<<" "<<position[1]<<" "<<position[2]<<std::endl;
              foundWires=false;
            }
          }
          int time = 3200. + traj->first[0]/vd/0.5 ; // [cm]/[cm/musec]/[musec/tick] ...  to within a few ticks this is true
          if(foundWires)
          {
            for(size_t plane=0;plane<3;++plane)
            {
              boost::multi_array<int, 2>  buffer(boost::extents[1][6]);
              buffer[0][0] = fNEvents;
              buffer[0][1] = tmp_instances++;
              buffer[0][2] = index;
              buffer[0][3] = plane;
              buffer[0][4] = wire[plane];
              buffer[0][5] = time;
              this->fNInstances++;
              std::cout<<wire[plane]<<" "<<time<<std::endl;


              fChunkDims[0] = this->fNInstances* this->fNEvents;
              hsize_t hyperslabSize[2] = { 1,6 };
              hsize_t newSize[2] = { this->fNInstances, 6 };
              this->fDataSet->extend( newSize );
              H5::DataSpace filespace(this->fDataSet->getSpace());
              hsize_t offset[2]={this->fNInstances-1,0};
              filespace.selectHyperslab( H5S_SELECT_SET, hyperslabSize, offset );
              H5::DataSpace memspace(2, hyperslabSize);

              std::cout<<"HDF5ParticleLabelSS: "<<this->fDataSetName<<" writing buffer to file" << std::endl;
              this->fDataSet->write( buffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
              std::cout<<"HDF5ParticleLabelSS: "<<this->fDataSetName<<" finished resetting buffer" << std::endl;

            }
          }
        }
      }
      else
      {
        std::cout<<"Found Particle Outside Label Table: "<<name<<std::endl;
      }
      
    }


  }

  
  void HDF5ParticleLabelSS::beginJob()
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    std::string group_name = "segments";
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,group_name,
      this->fDataSpace,
      this->fParms);

    H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
    H5::StrType strdatatype(H5::PredType::C_S1, 256);

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index0");
      const std::string strwritebuf ("event");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index1");
      const std::string strwritebuf ("instance");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index2");
      const std::string strwritebuf ("particle");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index3");
      const std::string strwritebuf ("plane");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index4");
      const std::string strwritebuf ("coordinate");

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
}





