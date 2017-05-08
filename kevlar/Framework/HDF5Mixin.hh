#ifndef HDF5MIXIN_HH
#define HDF5MIXIN_HH

#include "kevlar/Services/HDF5File.hh"
#include "fhiclcpp/ParameterSet.h"
#include <boost/multi_array.hpp>

namespace kevlar{

template<int Rank, class DataSetProperties>
class HDF5Converter{
    hsize_t fDims[Rank];
    hsize_t fMaxDims[Rank];
    hsize_t fChunkDims[Rank];
    H5::DataSpace fDataSpace;
    H5::DSetCreatPropList fParms;
    H5::DataSet* fDataSet;
    int fFillValue;
    uint32_t fNEvents;
    boost::multi_array<int, Rank>  fBuffer;
    uint32_t fBufferCounter;

  protected:
    kevlar::HDF5File* atDataSetSetup()
    {
      art::ServiceHandle<kevlar::HDF5File> _OutputFile;
      fDataSet = _OutputFile->CreateDataSet(this->fDataSetName,DataSetProperties::group,
        this->fDataSpace,
        this->fParms);
      return _OutputFile
    }

    void atBufferFill()
    {
      (this->fBufferCounter)++;
      (this->fNEvents)++;
      std::cout<<"HDF5Converter Buffer for "<<this->fDataSetName<<" :"<<this->fBufferCounter<<" Events"<<std::endl;

      if (this->fBufferCounter == this->fChunkDims[0]){

        hsize_t newSize[2] = {this->fNEvents,fDims[1]};
        this->fDataSet->extend( newSize );

        H5::DataSpace filespace(this->fDataSet->getSpace());

        hsize_t offset[2]={this->fNEvents-fChunkDims[0],0};
        filespace.selectHyperslab( H5S_SELECT_SET, this->fChunkDims, offset );

        H5::DataSpace memspace(2, fChunkDims);
        std::cout<<"HDF5Converter:"<<this->fDataSetName<<" writing buffer to file"<<std::endl;
        this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, 
                                memspace, filespace );
        this->fBufferCounter=0;
        fBuffer = boost::multi_array<int, 2>(boost::extents[fChunkDims[0]][fChunkDims[1]]);
        std::cout<<"HDF5Converter:"<<this->fDataSetName<<" finished resetting buffer"<<std::endl;
      }
    }

    void atDataSetCleanup()
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

  public:
    HDF5Converter(fhicl::ParameterSet const & pSet) :
      fDims{
        0, 
        fLabels.size(),
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
        fParms.setChunk( Rank, fChunkDims );
        fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
        fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",5));        
      }

};

}

#endif // HDF5MIXIN_HH
