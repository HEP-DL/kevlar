#include "kevlar/Converters/HDF5Image.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"

#include <boost/multi_array.hpp>

#include <fstream>
#include <string>
#include <iostream>
#include <vector>

namespace kevlar{

  HDF5Image::HDF5Image(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","rawdigits")),
      fDims{
        pSet.get<uint32_t>("ChunkSize",1),
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",6000),
      },
      fMaxDims{
        H5S_UNLIMITED,
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",6000),
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSize",1),
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",6000),        
      },
      fDataSpace(4, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0)
  {
      fParms.setChunk( 4, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
  }

  HDF5Image::~HDF5Image()
  {

  }

  void HDF5Image::analyze(art::Event const & evt)
  {
    boost::multi_array<int, 3>  _image(boost::extents[fDims[1]][fDims[2]][fDims[3]]);
    art::Handle<std::vector<raw::RawDigit> > digits;
    evt.getByLabel(fProducerName, digits);
    for (auto digitContainer: *digits){
      auto waveform = digitContainer.ADCs();
      auto channel = digitContainer.Channel();
      art::ServiceHandle<geo::Geometry> geo;
      auto wire = geo->ChannelToWire(channel).at(0).Wire;
      auto plane = geo->ChannelToWire(channel).at(0).Plane;

      std::cout<<plane<<","<<wire<<std::endl<<"\t";
      uint32_t tick=0;
      for(auto code: waveform){

        if(plane>=fDims[1] || tick>=fDims[2] || wire>=fDims[3]){
          std::cout<<"DIMENSIONS OUT OF ALIGNMENT: "<<std::endl;
          std::cout<<"("<<plane<<','<<tick<<','<<wire<<")"<<">=";
          std::cout<<"("<<fDims[0]<<','<<fDims[1]<<','<<fDims[2]<<")";
          std::cout<<std::endl;
          tick++;
          continue;
        }
        _image[plane][tick][wire] = code;
        ++tick;
      }
      std::cout<<std::endl;
    }

    hsize_t newSize[4] = {this->fNEvents+1,fDims[1],fDims[2],fDims[3]};

    this->fDataSet->extend( newSize );
    H5::DataSpace filespace(this->fDataSet->getSpace());
    hsize_t offset[4]={this->fNEvents,0,0,0};
    filespace.selectHyperslab( H5S_SELECT_SET, fChunkDims, offset );
    H5::DataSpace memspace(4, fChunkDims, NULL);
    this->fDataSet->write( _image.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    ++(this->fNEvents);
  }
  void HDF5Image::beginSubRun(art::SubRun const &)
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    std::string group_name = "image";
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,group_name,
      this->fDataSpace,
      this->fParms);
  }
  void HDF5Image::endSubRun(art::SubRun const & sr)
  {
  }
}