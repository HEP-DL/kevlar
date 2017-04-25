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

    unsigned int N;
    evt.getByLabel(fProducerName, digits);
    for (auto digitContainer: *digits){
      N = digitContainer.Samples();
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

    for (auto digitContainer: *digits){
      auto waveform = digitContainer.ADCs();
      auto channel = digitContainer.Channel();

      for(auto channel_spec : geo->ChannelToWire(channel)){

	uint32_t wire = channel_spec.Wire;
	uint32_t plane = channel_spec.Plane;

	uint32_t tick=0;
	std::chrono::nanoseconds LocNanoTime(fNanoTime);
	for(auto code: waveform){


	  /*
	  filename = basename + std::to_string(evt.run()) + "_" + std::to_string(evt.subRun()) + "_" + std::to_string(evt.event()) + "_";
	  filename += fTime + "_";
	  filename += std::to_string(plane) + ".dat";
	  */

	  // 8 fibers * 8 channels each carry 64 channels of one ADC ASIC from FEMB to WIB. 4 such ADC's worth of channels (256) fill one block.
	  // For MicroBooNE this leads to ~8256/256*9600 = 40k Frames, each about 256*1.5 ~ 400 Bytes large.
	  F.at(int(wire/256)).at(plane).at(tick).setCOLDATA(int(wire/64)%4, int(wire/8)%8, wire%8, int(code)); // block, fiber, channel
	  framegen::Frame Floc;
	  Floc.setCOLDATA(int(wire/64)%4, int(wire/8)%8, wire%8, int(code)); // block, fiber, channel
	  if (wire>250 && wire<=255 && tick==1111 && (plane==0||plane==2))
	    std::cout << "code is " << code << " for tick,plane,wire: " << tick <<", "<<plane<<", "<<wire << std::endl;
	  if ( ((wire+1)%256 == 0) || (wire == (uint32_t)wp.at(plane)) )
	    { // If we're in here we've filled a WIB Frame. We will set some header quantities and write it out.

	      F.at(int(wire/256)).at(plane).at(tick).setK28_5(0);
	      F.at(int(wire/256)).at(plane).at(tick).setVersion(2); // Version notation format subject to change.
	      F.at(int(wire/256)).at(plane).at(tick).setFiberNo(wire%8);
	      F.at(int(wire/256)).at(plane).at(tick).setCrateNo(wire%(512*5)); // flange?
	      F.at(int(wire/256)).at(plane).at(tick).setSlotNo(int(wire/512)); // Same as WIBCounter
	      //	      F.at(int(wire/256)).at(plane).at(tick).setWIBErrors(_randDouble(_mt)<_errProb);
	      F.at(int(wire/256)).at(plane).at(tick).setZ(0);
	      F.at(int(wire/256)).at(plane).at(tick).setTimestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(LocNanoTime).count()); 

	      Floc.setWIBCounter(int(wire/512));
	      Floc.setK28_5(0);
	      Floc.setVersion(2); // Version notation format subject to change.
	      Floc.setFiberNo(wire%8);
	      Floc.setCrateNo(wire%(512*5)); // flange?
	      Floc.setSlotNo(int(wire/512)); // Same as WIBCounter
	      //(plane).at(tick).setWIBErrors(_randDouble(_mt)<_errProb);
	      Floc.setZ(0);
	      Floc.setTimestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(LocNanoTime).count()); 

	      Floc.setWIBCounter(int(wire/512));
	      // For now let's write each Frame to its own file. 
	      // Later comment next 3 lines out to write one whole plane's worth of Frames to one file. EC, 15-Apr-2017.
	      filename = basename + "Run_" + std::to_string(evt.run()) + "-SubRun_" + std::to_string(evt.subRun()) + "-Event_" + std::to_string(evt.event()) + "-";
	      filename += "Plane_" + std::to_string(plane) + "-" ;
	      filename += "Tick_" + std::to_string(tick) + "-" ;
	      filename += "Frame_" + std::to_string(int(wire/256)) + ".dat";

	      F.at(int(wire/256)).at(plane).at(tick).resetChecksums();
	      Floc.resetChecksums();

	      if (wire==255 && tick==1111 && (plane==0||plane==2))
		std::cout << "writing " << filename << " for tick,plane,wire: " << tick <<", "<<plane<<", "<<wire << std::endl;

	      //	      F.at(int(wire/256)).at(plane).at(tick).print(filename, 'b'); // write the Frame to disk
	      Floc.print(filename, 'b'); // write the Frame to disk
	    }
	  tick++;
	  LocNanoTime += std::chrono::nanoseconds(500);
	} // code in waveform

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
