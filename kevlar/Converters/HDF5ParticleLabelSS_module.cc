#include "kevlar/Converters/HDF5ParticleLabelSS.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCTruth.h"
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
        0,
        fLabels.size(),
        3,
        2
      },
      //Index: [event, instance, particle, plane, coordinate]
      fMaxDims{
        H5S_UNLIMITED,
        H5S_UNLIMITED, 
        fLabels.size(),
        3,
        2
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSizeN",1),
        pSet.get<uint32_t>("ChunkSizeM",1),
        fLabels.size(),
        3,
        2
      },
      fDataSpace(5, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0),
      fNInstances(0)
  {
      fParms.setChunk( 5, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",5));
  }

  HDF5ParticleLabelSS::~HDF5ParticleLabelSS()
  {

  }

  void HDF5ParticleLabelSS::analyze(art::Event const & evt)
  {
    auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double vd = detprop->DriftVelocity(); //cm/musec
    art::ServiceHandle<geo::Geometry> geo;

    std::vector<std::vector<std::vector<std::pair<int, int> > > > eventBuffer;
    //indices are particle_type, plane, instance, coordinate
    for(size_t i=0; i<fLabels.size(); ++i)
    {
      eventBuffer.push_back(std::vector<std::vector<std::pair<int, int> > >());
      for(size_t j=0; j<3; ++j)
        eventBuffer[i].push_back(std::vector<std::pair<int, int> >() );
    }


    art::Handle< std::vector< simb::MCTruth > > mct_handle;
    std::cout<<"HDF5ParticleLabelVector:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle)
    {
      for(int i=0; i<truth.NParticles(); ++i)
      {
        int pdg = truth.GetParticle(i).PdgCode();
        auto particle = TDatabasePDG::Instance()->GetParticle(pdg);
        if(!particle)
          continue;
        std::string name = particle->GetName();
        ptrdiff_t index = std::find(fLabels.begin(), fLabels.end(), name) - fLabels.begin();

        if(index < int(fLabels.size()) && index>=0 )
        {
          std::cout<<"Found Particle with name: "<<name<< " and output vector index: "<<index<<std::endl;
          //Now that we have a particle, let's get a trajectory and use that to fill tree
          for (auto traj = truth.GetParticle(i).Trajectory().begin() ; traj != truth.GetParticle(i).Trajectory().end(); ++traj )
          {
            const TVector3 position = traj->first.Vect();
            unsigned* wire = new unsigned[3];
            bool foundWires=true;
            for(size_t plane=0;plane<3;plane++ )
            {
              try
              {
                wire[plane] = geo->NearestWire( position, plane);
              }
              catch(cet::exception& e )
              {
                foundWires=false;
              }
            }
            double time = 3200. + traj->first[0]/vd/0.5 ; // [cm]/[cm/musec]/[musec/tick] ...  to within a few ticks this is true

            if(foundWires)
            {
              for(size_t plane=0;plane<3;++plane)
              {
                eventBuffer.at(index).at(plane).push_back(std::make_pair(wire[plane], time));
                if(eventBuffer.at(index).at(plane).size()>this->fNInstances)
                {
                  this->fNInstances = eventBuffer.at(index).at(plane).size();
                  std::cout<<"HDF5ParticleLabelSS: Resizing Instance index to "<<this->fNInstances<<std::endl;
                }
              }
            }

            delete[] wire;
          }
        }
        else
        {
          std::cout<<"Found Particle Outside Label Table: "<<name<<std::endl;
        }
      }
    }
    //alright, now that we've got the event buffer, it's time to empty into the real buffer.
    //first, resize the real buffer if the dimensions are larger
    boost::multi_array<int, 5>  buffer(boost::extents[1][this->fNInstances][fLabels.size()][3][2]);
    (this->fNEvents)++;
    for(unsigned i=0; i < eventBuffer.size(); ++i)//particle type
    {
      for(unsigned j=0; j < eventBuffer[i].size(); ++j)//plane
      {
        for(unsigned k=0; k < eventBuffer[i][j].size(); ++k)//instance
        {
          buffer[0][k][i][j][0] = eventBuffer[i][j][k].first;
          buffer[0][k][i][j][1] = eventBuffer[i][j][k].second;
        }
      }
    }

    fChunkDims[1] = this->fNInstances;
    hsize_t hyperslabSize[5] = { 1, this->fNInstances, fLabels.size(), 3, 2 };
    hsize_t newSize[5] = { this->fNEvents, this->fNInstances, fLabels.size(), 3, 2 };
    this->fDataSet->extend( newSize );
    H5::DataSpace filespace(this->fDataSet->getSpace());
    hsize_t offset[5]={this->fNEvents-1,0,0,0,0};
    filespace.selectHyperslab( H5S_SELECT_SET, hyperslabSize, offset );
    H5::DataSpace memspace(5, hyperslabSize);

    std::cout<<"hyperslab size: ";
    for(int i=0; i<5;i++)std::cout<<hyperslabSize[i]<<" ";
      std::cout<<std::endl;

    std::cout<<"new size: ";
    for(int i=0; i<5;i++)std::cout<<newSize[i]<<" ";
      std::cout<<std::endl;

    std::cout<<"offset: ";
    for(int i=0; i<5;i++)std::cout<<offset[i]<<" ";
      std::cout<<std::endl;

    std::cout<<this->fDataSet<<std::endl;

    std::cout<<"Semantic Segmenter:"<<this->fDataSetName<<" writing buffer to file" << std::endl;
    this->fDataSet->write( buffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    std::cout<<"Semantic Segmenter:"<<this->fDataSetName<<" finished resetting buffer" << std::endl;
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
