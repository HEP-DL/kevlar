#include "kevlar/WIB/ColdataValidation.hh"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

//#include "art/Framework/IO/Sources/put_product_in_principal.h"


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


  ColdataValidation::ColdataValidation(fhicl::ParameterSet const & pSet):
    //      art::EDProducer(pSet),
    fNEvents(0),
    fFilePath(pSet.get<std::string>("FilePath","."))
  {

    time_t rawtime;
    //    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    auto t = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",t);
    fTime = std::string(buffer);

    fNanoTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

    produces< std::vector<raw::RawDigit>   >();



  }

  ColdataValidation::~ColdataValidation()
  {

  }

  void ColdataValidation::produce(art::Event  & evt)
  {

    std::unique_ptr<std::vector<raw::RawDigit> > rawdigit_ptr(new std::vector<raw::RawDigit> );

    
    art::ServiceHandle<geo::Geometry> geo;
    const unsigned int Nticks(9600);



    std::vector<int> wp ; wp.reserve(geo->Nviews());
    for (const auto &v : geo->Views())
      {
	wp.push_back(geo->Nwires(v));
      }

    //    const int MaxWiresPerPlane( *std::max_element(wp.begin(),wp.end()) );    

    // 8 fibers * 8 channels each carry 64 channels of one ADC ASIC from FEMB to WIB. 4 such ADC's worth of channels (256) fill one block.
    // For MicroBooNE this leads to ~8256/256*9600 = 40k Frames, each about 256*1.5 ~ 400 Bytes large.


    uint16_t channel(0);

    for (int32_t plane=0;  (uint32_t)plane<wp.size(); plane++) {
      std::vector <std::vector <int16_t> > digidos ;
      std::vector <int16_t> digit(Nticks);

      for (uint32_t ii=0; ii<(uint32_t)wp[plane]; ii++)
	digidos.push_back(digit);

      std::vector <uint32_t> W(Nticks,0); 
      for (uint32_t tick=0; tick<Nticks ; tick++) {
	
	for (uint32_t f=0; f<(uint32_t)(wp.at(plane)%256) ; f++) {
	  // Read a file holding one block.
	  framegen::Frame F;

	  std::string filename(fFilePath);
	  std::string basename("/WIBFrameFile-");

	  bool badfile(false);
	  try{

	    filename += basename + "Run_" + std::to_string(evt.run()) + "-SubRun_" + std::to_string(evt.subRun()) + "-Event_" + std::to_string(evt.event()) + "-";
	    filename += "Plane_" + std::to_string(plane) + "-" ;
	    filename += "Tick_" + std::to_string(tick) + "-" ;
	    filename += "Frame_" + std::to_string(f) + ".dat";
	    badfile = !F.load(filename,0); // only 1 Frame per file, so 0

	  }
	  catch (...) {
	    std::cout << "No Filename: " << filename << ". Moving onto next frame, then next tick, then plane. " <<std::endl;
	    badfile = true;
	  }
	  if (badfile) break;
	  
	  const uint16_t wibcnt = F.getWIBCounter();
	  // const uint8_t k28 = F.getK28_5();
	  const uint8_t fib = F.getFiberNo();
	  const uint8_t crate = F.getCrateNo(); // flange?
	  const uint8_t slot = F.getSlotNo(); // Same as WIBCounter
	  // const uint8_t z = F.getZ();
	  const uint64_t time = F.getTimestamp(); 
	  
	  if (!(tick%1000)) {
	    std::cout << "ColdataValidation_module: Reading " << filename << std::endl;
	    std::cout << "                          WIB, Fiber, Crate, Slot: " << std::to_string(wibcnt) << ", " << std::to_string(fib) << ", " << std::to_string(crate) << ", " << std::to_string(slot) << std::endl;
	    std::cout << "                          Timestamp: " << std::to_string(time) << std::endl;
	  }

	  for (uint32_t wire=0; wire<256; wire++) {
	    int adc = F.getCOLDATA(wire%64, int(wire/8)%8, wire%8);
	    //	    std::cout << " adc, wire, tick, W.at(tick), digidos.size(), digidos.at(W.at(tick)).size()" << adc << ", " << wire << ", " << tick << ", " << W.at(tick) << ", " << digidos.size() <<", " << digidos.at(W.at(tick)).size()  << std::endl;
	    digidos.at(W.at(tick)).at(tick) = adc;
	    W.at(tick)++;
	  }

	} // maxwiresperplane%256  -- blocks


      } // ticks

      for (uint16_t ii=0; ii<wp.at(plane); ii++) {
	//  raw::RawDigit::RawDigit(raw::ChannelID_t, short unsigned int, const ADCvector_t&, raw::Compress_t)
	rawdigit_ptr->emplace_back( (raw::ChannelID_t) channel, (uint16_t) digidos.at(ii).size(), std::move(digidos.at(ii)), raw::kNone);
	channel += ii; 
      }

    } // planes
    
    evt.put(std::move(rawdigit_ptr));

    //    art::put_product_in_principal(std::move(rawdigit_ptr),
    //				  &evt,
    //				  "wibval"); // Module label


  }
  
  void ColdataValidation::beginJob()
  {
  }

  void ColdataValidation::endJob()
  {
    //      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    
  }


}
