#include "kevlar/Converters/HDF5Label.hh"
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

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>


namespace kevlar{

  HDF5Label::HDF5Label(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","type")),
      fLabels(pSet.get< std::vector<std::string> >("labels")),
      fDims{
        pSet.get<uint32_t>("ChunkSize",1), fLabels.size(),
      },
      fMaxDims{
        H5S_UNLIMITED, fLabels.size(),
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSize",1), fLabels.size()
      },
      fDataSpace(2, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0),
      fBuffer(boost::extents[fChunkDims[0]][fChunkDims[1]]),
      fBufferCounter(0)
  {
      fParms.setChunk( 2, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",9));
  }

  HDF5Label::~HDF5Label()
  {

  }

  void HDF5Label::analyze(art::Event const & evt)
  {
    
    art::Handle< std::vector< simb::MCTruth > > mct_handle;

    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle){
      for(int i=0; i<truth.NParticles(); ++i){
        int pdg = truth.GetParticle(i).PdgCode();
	if (abs(pdg) > 1E7) continue; // don't try GetName() on the Argon nucleus, e.g.
        std::string name = TDatabasePDG::Instance()->GetParticle(pdg)->GetName();

        ptrdiff_t index = std::find(fLabels.begin(), fLabels.end(), name) - fLabels.begin();
        if(index < int(fLabels.size()) ){
          fBuffer[fBufferCounter][index] = fBuffer[fBufferCounter][index] + 1;
        }
        else{
          std::cout<<"Found Particle Outside Label Table: "<<name<<std::endl;          
        }
      }
    }

    (this->fBufferCounter)++;
    (this->fNEvents)++;

    if (this->fBufferCounter == this->fChunkDims[0]){

      hsize_t newSize[2] = {this->fNEvents,fDims[1]};
      this->fDataSet->extend( newSize );

      H5::DataSpace filespace(this->fDataSet->getSpace());

      hsize_t offset[2]={this->fNEvents-fChunkDims[0],0};
      filespace.selectHyperslab( H5S_SELECT_SET, this->fChunkDims, offset );

      H5::DataSpace memspace(2, fChunkDims);
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, 
                              memspace, filespace );
      this->fBufferCounter=0;
      fBuffer = boost::multi_array<int, 2>(boost::extents[fChunkDims[0]][fChunkDims[1]]);
    }

  }
  
  void HDF5Label::beginSubRun(art::SubRun const &)
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    std::string group_name = "label";
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,group_name,
      this->fDataSpace,
      this->fParms);

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

  void HDF5Label::endSubRun(art::SubRun const & sr)
  {
    if(!(this->fBufferCounter==0)){
      // The new size is now the number of  events in the file
      hsize_t newSize[2] = {this->fNEvents,fDims[1]};
      this->fDataSet->extend( newSize );

      //The filespace is always the same.
      H5::DataSpace filespace(this->fDataSet->getSpace());

      // The offset is the number of events less the size of the buffer
      hsize_t offset[2] = {this->fNEvents-fBufferCounter,0};

      // Leftovers are the size of the buffer, not the chunk
      hsize_t leftover_size[2] = { this->fBufferCounter, fChunkDims[1] };
      filespace.selectHyperslab( H5S_SELECT_SET, leftover_size, offset );

      //Select a memoty space for the data to use
      H5::DataSpace memspace(2, leftover_size);

      // Now write and reset
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
    }
  }
}
