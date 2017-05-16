#include "kevlar/WIB/Coldata.hh"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "lardata/DetectorInfoServices/DetectorClocksServiceStandard.h" // FIXME: this is not portable    
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "TDatabasePDG.h"
#include "FrameGen/FrameGen.hpp"

#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOneP.h"


#include <exception>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>  // put_time
#include <ctime>
#include <chrono>

namespace kevlar{

  class MotherNotFound: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "Primary Mother particle not found.";
    }
  };

  Coldata::Coldata(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","daq")),
      fG4Name(pSet.get<std::string>("G4Label","largeant")),
      fNEvents(0)
  {

    /*  needs gcc5+
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    fTime = oss.str();
    */

    time_t rawtime;
    //    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    auto t = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",t);
    fTime = std::string(buffer);

    fNanoTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

  }

  Coldata::~Coldata()
  {

  }

  void Coldata::analyze(art::Event const & evt)
  {
    
    art::ServiceHandle<geo::Geometry> geo;
    //TimeService
    //    art::ServiceHandle<detinfo::DetectorClocksServiceStandard> tss;

    //    tss->preProcessEvent(evt);
    // auto const* ts = tss->provider();

    //    auto const* detprop = lar::providerFrom<detinfo::DetectorPropertiesService>();
    //    double vd = detprop->DriftVelocity(); //cm/musec

    art::Handle<std::vector<raw::RawDigit> > digits;
    std::vector<raw::RawDigit> prd;

    unsigned int N;
    evt.getByLabel(fProducerName, digits);
    for (auto digitContainer: *digits){
      N = digitContainer.Samples();
      prd.push_back(digitContainer);
      break;
    }
    const unsigned int Nticks(N);

    // Let's write to one of N ColdData_blocks: 
    // One event's worth of samples (9600 x 3) x ((Nw%256)+1) are needed
    // 9600 uB ticks, 3 planes, Nw per plane. Each Coldata WIB block holds 256 channels.



    framegen::Frame   Fbare ;
    std::vector< std::vector< std::vector<framegen::Frame> > > F;

    std::vector<framegen::Frame>   Ft; 
    Ft.reserve( Nticks ) ;
    std::vector< std::vector<framegen::Frame> >  Fv; 
    Fv.reserve ( geo->Nviews() );

    for (unsigned int ii=0; ii<Nticks; ii++) 
    {
      Ft.push_back(Fbare);
    }
    std::vector<int> wp ; wp.reserve(geo->Nviews());
    for (const auto &v : geo->Views())
      {
	Fv.push_back(Ft);
	wp.push_back(geo->Nwires(v));
      }

    const int MaxWiresPerPlane( *std::max_element(wp.begin(),wp.end()) );    

    F.reserve(int(MaxWiresPerPlane/256)+1);
    for (int ii=0; ii<(int) (MaxWiresPerPlane/256) + 1; ii++)
      {
	F.push_back(Fv);
      }
    // Now we've got all our needed Frames.

    std::string filename("");
    std::string basename("./WIBFrameFile-");

    // This has to grab 256 rawdigits at a time, not each one. Then shove those into 256 vectors. EC, 28-Apr-2017
    // Then in outer loop for each time tick loop on those 256 rawdigits at that tick and shove into block.

    
    std::vector<std::vector<short int>> codeBlk;
    uint32_t ch(0);

    for (auto digitContainer: *digits){ 
      
      auto waveform = digitContainer.ADCs();
      auto channel = digitContainer.Channel();

      codeBlk.push_back(waveform);

      // This in uB is a simple 1-to-1 map. Not so in DUNE.
      for(auto channel_spec : geo->ChannelToWire(channel)){

	uint32_t wire_outer = channel_spec.Wire ;
	uint32_t plane_outer = channel_spec.Plane;

	//	std::cout << "Coldata_module: wire_outer, plane_outer " << wire_outer << ", " << plane_outer << std::endl ;
	// 8 fibers * 8 channels each carry 64 channels of one ADC ASIC from FEMB to WIB. 4 such ADC's worth of channels (256) fill one block.
	  // For MicroBooNE this leads to ~8256/256*9600 = 40k Frames, each about 256*1.5 ~ 400 Bytes large.

	if ( ((codeBlk.size())%256 == 0) || (wire_outer == (uint32_t)(wp.at(plane_outer)-1) )  )
	  { // If we're in here we've filled a codeBlk to go into a WIB Frame. We will set some header quantities and write it out.

	    std::cout << "wire_outer number: " << wire_outer << std::endl;
	    uint32_t tick=0; 
	    uint32_t upper;
	    if ( (codeBlk.size())%256 == 0 ) upper=256;
	    else { // This means we've hit last wire on plane and want to move onto a new slot, I think.
	      upper = wire_outer%256;
	    }

	    while (tick < Nticks) {
	      std::chrono::nanoseconds LocNanoTime(500*tick);
	      framegen::Frame Floc;
	      Floc.setWIBCounter(int(wire_outer/512));
	      Floc.setK28_5(0);
	      Floc.setVersion(2); // Version notation format subject to change.
	      Floc.setFiberNo(wire_outer%8); // what is this? We've agreed it takes 8 fibers x 4 per Frame, right?
	      Floc.setCrateNo(int(wire_outer/(512*5))); // flange?
	      Floc.setSlotNo(int(wire_outer/512)); // Same as WIBCounter
	      //(plane).at(tick).setWIBErrors(_randDouble(_mt)<_errProb);
	      Floc.setZ(0);
	      Floc.setTimestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(fNanoTime+LocNanoTime).count()); 
	      
	      for (uint32_t wire=0; wire<upper; wire++)
		{
		    
		  Floc.setCOLDATA(int(wire/64), int(wire/8)%8, wire%8, int(codeBlk.at(wire).at(tick)) ); // block, fiber, channel		  
		  // std::cout << "tick/wire/wire_outer number/adc: " << tick << "/" << wire << "/" << wire_outer << "/" << int(codeBlk.at(wire).at(tick)) << std::endl;
		  
		} // loop over latest 256 wires
	      Floc.resetChecksums();
	      // For now let's write each Frame to its own file. 
	      // Later comment next 3 lines out to write one whole plane's worth of Frames to one file. EC, 15-Apr-2017.
	      filename = basename + "Run_" + std::to_string(evt.run()) + "-SubRun_" + std::to_string(evt.subRun()) + "-Event_" + std::to_string(evt.event()) + "-";
	      filename += "Plane_" + std::to_string(plane_outer) + "-" ;
	      filename += "Tick_" + std::to_string(tick) + "-" ;
	      filename += "Frame_" + std::to_string(int(ch/256)) + ".dat";

	      Floc.print(filename, 'b'); // write the Frame to disk

	      if (!tick) ch+=256;
	      tick++;
	      LocNanoTime += std::chrono::nanoseconds(500);
		
	    } // while -- value at each tick -- in waveform
	    codeBlk.clear();
	  } // if we're in 256 wire or last one on plane

	if (wire_outer == (uint32_t)(wp.at(plane_outer)-1) ) ch = 0;

      } // channel_spec

    } // digit_container
      
  }
  
  void Coldata::beginJob()
  {
  }

  void Coldata::endJob()
  {
    //      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    
  }


}
