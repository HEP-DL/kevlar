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

#include <boost/multi_array.hpp>

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>


namespace kevlar{

  HDF5Label::HDF5Label(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","label/type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels")),
      fDims{
        pSet.get<uint32_t>("ChunkSize",1),
        fLabels.size(),
      },
      fMaxDims{
        H5S_UNLIMITED,
        fLabels.size(),
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSize",1),
        fLabels.size()
      },
      fDataSpace(2, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0)
  {
      fParms.setChunk( 2, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
  }

  HDF5Label::~HDF5Label()
  {

  }

  void HDF5Label::analyze(art::Event const & evt)
  {
    boost::multi_array<int, 1>  _label_vector(boost::extents[this->fLabels.size()]);
    
    art::Handle< std::vector< simb::MCTruth > > mct_handle;

    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle){
      for(int i=0; i<truth.NParticles(); ++i){
        int pdg = truth.GetParticle(i).PdgCode();
        std::string name = TDatabasePDG::Instance()->GetParticle(pdg)->GetName();

        ptrdiff_t index = std::find(fLabels.begin(), fLabels.end(), name) - fLabels.begin();
        if(index < int(fLabels.size()) ){
          _label_vector[index] = _label_vector[index] + 1;
        }
        else{
          std::cout<<"Found Particle Outside Label Table: "<<name<<std::endl;          
        }
      }
    }

    H5::DataSpace filespace(this->fDataSet->getSpace ());
    hsize_t offset[2]={this->fNEvents,0};
    filespace.selectHyperslab( H5S_SELECT_SET, fChunkDims, offset );
    H5::DataSpace memspace(2, fChunkDims, NULL);
    this->fDataSet->write( _label_vector.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    ++(this->fNEvents);
  }
  void HDF5Label::beginSubRun(art::SubRun const &)
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,
      this->fDataSpace,
      this->fParms);
  }
  void HDF5Label::endSubRun(art::SubRun const & sr)
  {
  }
}