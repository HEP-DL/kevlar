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


namespace kevlar{

  HDF5Label::HDF5Label(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels")),
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
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",7));
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
    ++(this->fBufferCounter);

    if (this->fBufferCounter == this->fChunkDims[0]){
      hsize_t newSize[2] = {this->fNEvents+1,fDims[1]};
      this->fDataSet->extend( newSize );
      H5::DataSpace filespace(this->fDataSet->getSpace());
      hsize_t offset[2]={this->fNEvents-fChunkDims[0]+1,0};
      filespace.selectHyperslab( H5S_SELECT_SET, fChunkDims, offset );
      H5::DataSpace memspace(2, fChunkDims, NULL);
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
      fBuffer = boost::multi_array<int, 2>(boost::extents[fChunkDims[0]][fChunkDims[1]]);
    }

    ++(this->fNEvents);

  }
  
  void HDF5Label::beginSubRun(art::SubRun const &)
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    std::string group_name = "label";
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,group_name,
      this->fDataSpace,
      this->fParms);
  }

  void HDF5Label::endSubRun(art::SubRun const & sr)
  {
    if(!(this->fBufferCounter==0)){
      hsize_t newSize[2] = {this->fNEvents+1,fDims[1]};
      this->fDataSet->extend( newSize );
      H5::DataSpace filespace(this->fDataSet->getSpace());
      hsize_t offset[2]={this->fNEvents-fBufferCounter,0};
      filespace.selectHyperslab( H5S_SELECT_SET, fChunkDims, offset );
      H5::DataSpace memspace(4, fChunkDims, NULL);
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
    }
  }
}