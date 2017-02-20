#include "kevlar/Converters/HDF5Label.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <boost/multi_array.hpp>

#include <fstream>
#include <string>
#include <iostream>


namespace kevlar{

  HDF5Label::HDF5Label(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","label/type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels")),
      fDims{
        H5S_UNLIMITED,
        fLabels.size(),
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSize",10),
        fLabels.size()
      },
      fDataSpace(2, fDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0)
  {
      fParms.setChunk( 4, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
  }

  HDF5Label::~HDF5Label()
  {

  }

  void HDF5Label::analyze(art::Event const & evt)
  {
    boost::multi_array<double, 3>  _label_vector(boost::extents[this->fLabels.size()]);
    
    art::Handle< std::vector< simb::MCTruth > > mct_handle;

    evt.getByLabel(fProducerName, mct_handle);
    for (auto truth: *mct_handle){
      for(unsigned i=0; i<truth.NParticles(); ++i){
        uint32_t pdg = mct_handle.GetParticle(i).PdgCode();
        std::string name = TDatabasePDG::Instance()->GetParticle(pdg)->GetName();

        auto index = std::find(fLabels.begin(), fLabels.end(), name);
        if(index<= fLabels.size()){

        }
        else{
        }
      }
    }







    for (auto digitContainer: *digits){
      auto waveform = digitContainer.ADCs();
      auto channel = digitContainer.Channel();
      art::ServiceHandle<geo::Geometry> geo;
      auto wire = geo->ChannelToWire(channel).at(0).Wire;
      auto plane = geo->ChannelToWire(channel).at(0).Plane;

      std::cout<<plane<<","<<wire<<std::endl<<"\t";
      uint32_t tick=0;
      for(auto code: waveform){
        std::cout<<"("<<tick<<","<<code<<"),";

        if(plane>=fDims[0] || tick>=fDims[1] || wire>=fDims[2]){
          std::cout<<"DIMENSIONS OUT OF ALIGNMENT: "<<std::endl;
          std::cout<<"("<<plane<<','<<tick<<','<<wire<<")"<<">=";
          std::cout<<"("<<fDims[0]<<','<<fDims[1]<<','<<fDims[2]<<")";
          tick++;
          continue;
        }
        _image[plane][tick][wire] = code;
        ++tick;
      }
      std::cout<<std::endl;
    }

    hsize_t offset[4]={this->fNEvents,0,0,0};
    this->fDataSpace.selectHyperslab( H5S_SELECT_SET, fChunkDims, offset );
    this->fDataSet->write( _image.data(), H5::PredType::NATIVE_INT, this->fDataSpace, this->fDataSpace );
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