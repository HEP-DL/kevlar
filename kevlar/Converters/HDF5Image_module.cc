#include "kevlar/Converters/HDF5Image.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/RawDigit.h"

#include "H5Cpp.h"

#include <fstream>
#include <string>
#include <iostream>

namespace kevlar{

  HDF5Image::HDF5Image(fhicl::ParameterSet const & pSet):
      art::EDAnalyzer(pSet),
      fProducerName(pSet.get<std::string>("ProducerLabel","largeant"))
  {

  }
  HDF5Image::~HDF5Image()
  {

  }
  void HDF5Image::analyze(art::Event const & evt)
  {
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
        //now we've got wire, tick and code (x,y and pixel value)
        //also, we've got plane ()
        std::cout<<"("<<tick<<","<<code<<"),";

        ++tick;
      }
      std::cout<<std::endl;
    }
  }
  void HDF5Image::beginSubRun(art::SubRun const & sr)
  {

  }
  void HDF5Image::endSubRun(art::SubRun const & sr)
  {

  }

}