#include "kevlar/Converters/HDF5VertexPlaneProjection.hh"
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

#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOneP.h"


#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>


namespace kevlar{

  class MotherNotFound: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "Primary Mother particle not found.";
    }
  };

  HDF5VertexPlaneProjection::HDF5VertexPlaneProjection(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","generator")),
      fG4Name(pSet.get<std::string>("G4Label","largeant")),
      fDataSetName(pSet.get<std::string>("DataSetLabel","type")),
      fLabels(pSet.get< std::vector<std::string> >("Labels")),
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
      fParms.setChunk( 2, fChunkDims );
      fParms.setFillValue( H5::PredType::NATIVE_INT, &fFillValue);
      fParms.setDeflate(pSet.get<uint32_t>("CompressionLevel",5));
  }

  HDF5VertexPlaneProjection::~HDF5VertexPlaneProjection()
  {

  }

  void HDF5VertexPlaneProjection::analyze(art::Event const & evt)
  {
    
    art::ServiceHandle<geo::Geometry> geo;

    auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
    double vd = detprop->DriftVelocity(); //cm/musec

    art::Handle< std::vector< simb::MCTruth > > mct_handle;

    std::cout<<"HDF5VertexPlaneProjection:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
    evt.getByLabel(fProducerName, mct_handle);
    art::FindManyP<simb::MCParticle> assnMCP(mct_handle, evt, fG4Name);
    //find origin. Somehow I thnk assnMCP is a vector, not a v-of-v's
    std::vector< art::Ptr<simb::MCParticle> > mcpdks = assnMCP.at(0);

    bool mother(false);

    for (auto truth: *mct_handle){
      for(size_t i=0; i<(size_t)truth.NParticles(); ++i){
        int pdg = truth.GetParticle(i).PdgCode();
        if (abs(pdg) > 1E7) continue; // don't try GetName() on the Argon nucleus, e.g.

        auto particle = TDatabasePDG::Instance()->GetParticle(pdg);
        if(!particle)
          continue;
        std::string name = particle->GetName();

        int Wire[3] = {0,0,0};


        for (auto const& mcpdk : mcpdks ) {
	  std::cout<<"HDF5VertexPlaneProjection:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
	  std::cout<<"HDF5VertexPlaneProjection: Process() of this particle: :"<<  mcpdk->Process() <<std::endl;
	  if ( mcpdk->Process() == "primary" )
	    {
	      //	      std::cout<<"Found particle: "<<pdg<<" "<<truth.GetParticle(i).Process()<<std::endl;
	      TLorentzVector xyzt = mcpdk->Position();
	      const TVector3 xyz = mcpdk->Position().Vect();
	      bool meh(false);
	      for (size_t ii=0; ii<3; ii++) 
		{

		  try
		    {		    
		      Wire[ii] = geo->NearestWire( xyz, ii);
		    }
		  catch(cet::exception& e )
		    {
		      meh = true;
		    }
		  if (!meh)
		    std::cout << "ii, xyz, Wire are " << ii << ", " << xyz[ii] << ", " << Wire[ii] << std::endl;
		}
	      
	      double Time = 3200.;
	      if (!meh)
		Time += xyzt[0]/vd/0.5 ; // [cm]/[cm/musec]/[musec/tick] ...  to within a few ticks this is true
              
        for (int index=0; index<3; index++ )
          fBuffer[fBufferCounter][index] = (double) Wire[index];
        fBuffer[fBufferCounter][3] = Time;
        mother = true;
        break; // we only want the one pdk info
      } //primary
        } // mcpdks
        if (mother) break;
      } // truth particles
      if (mother) break;
    }   // truth bundles in handle

    if(! mother){
      std::cerr<<"Problem in HDF5VertexPlaneProjection No primary Mother particle found. Throwing. "<< std::endl;
      throw MotherNotFound();
    }

    else{
      std::cout << "";
      for (int index=0; index<4; index++ )
  std::cout << fBuffer[fBufferCounter][index] << ", ";
      std::cout << "" << std::endl;
    }

    (this->fBufferCounter)++;
    (this->fNEvents)++;
    std::cout<<"HDF5VertexPlaneProjection Buffer for "<<this->fDataSetName<<" : "<<this->fBufferCounter<<" Events"<<std::endl;

    if (this->fBufferCounter == this->fChunkDims[0]){

      hsize_t newSize[2] = {this->fNEvents,fDims[1]};
      this->fDataSet->extend( newSize );

      H5::DataSpace filespace(this->fDataSet->getSpace());

      hsize_t offset[2]={this->fNEvents-fChunkDims[0],0};
      filespace.selectHyperslab( H5S_SELECT_SET, this->fChunkDims, offset );

      H5::DataSpace memspace(2, fChunkDims);
      std::cout<<"HDF5VertexPlaneProjection:"<<this->fDataSetName<<" writing buffer to file"<<std::endl;
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, 
                              memspace, filespace );
      this->fBufferCounter=0;
      fBuffer = boost::multi_array<int, 2>(boost::extents[fChunkDims[0]][fChunkDims[1]]);
      std::cout<<"HDF5VertexPlaneProjection:"<<this->fDataSetName<<" finished resetting buffer"<<std::endl;
    }

  }
  
  void HDF5VertexPlaneProjection::beginJob()
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
      const std::string ATTR_NAME ("coord_origin");
      const std::string strwritebuf ("primary_uvyt_origin");

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

  void HDF5VertexPlaneProjection::endJob()
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

      //Select a memory space for the data to use
      H5::DataSpace memspace(2, leftover_size);

      // Now write and reset
      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
      this->fBufferCounter=0;
    }
  }
}
