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

  Coldata::Coldata(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","generator")),
      fG4Name(pSet.get<std::string>("G4Label","largeant")),
      fNEvents(0)
  {
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

    art::Handle< std::vector< simb::MCTruth > > mct_handle;

    //    std::cout<<"Coldata:"<<this->fDataSetName<<" reading into buffer"<<std::endl;
    evt.getByLabel(fProducerName, mct_handle);
    art::FindManyP<simb::MCParticle> assnMCP(mct_handle, evt, fG4Name);
    //find origin. Somehow I thnk assnMCP is a vector, not a v-of-v's
    std::vector< art::Ptr<simb::MCParticle> > mcpdks = assnMCP.at(0);



    //    std::cout<<"Coldata:"<<this->fDataSetName<<" finished resetting buffer"<<std::endl;


  }
  
  void Coldata::beginJob()
  {
  }

  void Coldata::endJob()
  {
    //      this->fDataSet->write( fBuffer.data(), H5::PredType::NATIVE_INT, memspace, filespace );
    
  }


}
