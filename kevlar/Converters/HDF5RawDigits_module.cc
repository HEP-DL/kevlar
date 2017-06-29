#include "kevlar/Converters/HDF5RawDigits.hh"
#include "kevlar/Services/HDF5File.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"

#include <fstream>
#include <string>
#include <iostream>
#include <vector>

namespace kevlar{

  HDF5RawDigits::HDF5RawDigits(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","daq")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","rawdigits")),
      fDims{
        0,//start with no space
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",3456),
      },
      fMaxDims{
        H5S_UNLIMITED,
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",3456),
      },
      fChunkDims{
        pSet.get<uint32_t>("ChunkSize",1),
        pSet.get<uint32_t>("NChannels",3),
        pSet.get<uint32_t>("ImageHeight",9600),
        pSet.get<uint32_t>("ImageWidth",3456),        
      },
      fDataSpace(4, fDims, fMaxDims),
      fParms(),
      fDataSet(NULL),
      fFillValue(pSet.get<uint32_t>("FillValue",0)),
      fNEvents(0),
      fBuffer(boost::extents[fChunkDims[0]][fChunkDims[1]][fChunkDims[2]][fChunkDims[3]]),
      fBufferCounter(0)
  {
      fParms.setChunk( 4, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",5));
      std::cout<<"Finished with HDF5RawDigits default c'tor for module: "<<this->fDataSetName<<std::endl;
  }

  HDF5RawDigits::~HDF5RawDigits()
  {

  }

  void HDF5RawDigits::analyze(art::Event const & evt)
  {

    //Grab the rela
    art::Handle<std::vector<raw::RawDigit> > digits;
    art::ServiceHandle<geo::Geometry> geo;
    std::cout<<"HDF5RawDigits:"<<this->fDataSetName<<" reading into buffer"<<std::endl;

    evt.getByLabel(fProducerName, digits);
    for (auto digitContainer: *digits){
      auto waveform = digitContainer.ADCs();
      auto channel = digitContainer.Channel();

      for(auto channel_spec : geo->ChannelToWire(channel)){

        uint32_t wire = channel_spec.Wire;
        uint32_t plane = channel_spec.Plane;
        uint32_t tick=0;
        for(auto code: waveform){

          if(plane>=fDims[1] || tick>=fDims[2] || wire>=fDims[3]){
            std::cerr<<"DIMENSIONS OUT OF ALIGNMENT: "<<std::endl;
            std::cerr<<"("<<plane<<','<<tick<<','<<wire<<")"<<">=";
            std::cerr<<"("<<fDims[0]<<','<<fDims[1]<<','<<fDims[2]<<")";
            std::cerr<<std::endl;
            tick++;
            continue;
          }
          fBuffer[fBufferCounter][plane][tick][wire] = int(code) ;
          ++tick;
        }
      }
    }


    (this->fBufferCounter)++;
    (this->fNEvents)++;

    std::cout<<"HDF5RawDigits Buffer for "<<this->fDataSetName<<" :"<<this->fBufferCounter<<" Events"<<std::endl;

    if (this->fBufferCounter == this->fChunkDims[0]){
      hsize_t newSize[4] = {this->fNEvents,fDims[1],fDims[2],fDims[3]};
      this->fDataSet->extend( newSize );

      H5::DataSpace filespace(this->fDataSet->getSpace());

      hsize_t offset[4]={ this->fNEvents-fChunkDims[0] , 0, 0, 0};
      filespace.selectHyperslab( H5S_SELECT_SET, this->fChunkDims, offset );

      H5::DataSpace memspace(4, this->fChunkDims);
      std::cout<<"HDF5RawDigits:"<<this->fDataSetName<<" writing buffer to file"<<std::endl;
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
      fBuffer = boost::multi_array<int, 4>(boost::extents[fChunkDims[0]][fChunkDims[1]][fChunkDims[2]][fChunkDims[3]]);
      std::cout<<"HDF5RawDigits:"<<this->fDataSetName<<" finished resetting buffer"<<std::endl;
    }

  }
  
  void HDF5RawDigits::beginJob()
  {
    art::ServiceHandle<kevlar::HDF5File> _OutputFile;
    std::string group_name = "image";
    fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,group_name,
      this->fDataSpace,
      this->fParms);

    // Now setup some attributes

     H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);
     H5::StrType strdatatype(H5::PredType::C_S1, 256);

     {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index0");
      const std::string strwritebuf ("eventN");

      // Create attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index1");
      const std::string strwritebuf ("planeN");

      // Create H5::attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index2");
      const std::string strwritebuf ("timeT");

      // Create H5::attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

    {
      // Set up write buffer for attribute
      const std::string ATTR_NAME ("index3");
      const std::string strwritebuf ("wireN");

      // Create H5::attribute and write to it
      H5::Attribute myatt_in = this->fDataSet->createAttribute(ATTR_NAME, strdatatype, attr_dataspace);
      myatt_in.write(strdatatype, strwritebuf); 
    }

  }

  void HDF5RawDigits::endJob()
  {

    if(!(this->fBufferCounter==0)){
      // The new size is now the number of  events in the file
      hsize_t newSize[4] = {this->fNEvents,fDims[1],fDims[2],fDims[3]};
      this->fDataSet->extend( newSize );

      //The filespace is always the same.
      H5::DataSpace filespace(this->fDataSet->getSpace());

      // The offset is the number of events less the size of the buffer
      hsize_t offset[4] = {this->fNEvents-fBufferCounter,0,0,0};

      // Leftovers are the size of the buffer, not the chunk
      hsize_t leftover_size[4] = { this->fBufferCounter, fChunkDims[1],fChunkDims[2],fChunkDims[3] };
      filespace.selectHyperslab( H5S_SELECT_SET, leftover_size, offset );

      //Select a memoty space for the data to use
      H5::DataSpace memspace(4, leftover_size);

      // Now write and reset
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
    }
  }
}
