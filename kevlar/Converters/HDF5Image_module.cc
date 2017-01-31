#include "kevlar/Converters/HDF5Image.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h"

#include "H5Cpp.h"

#include <fstream>
#include <string>


namespace kevlar{

  HDF5Image::HDF5Image(fhicl::ParameterSet const & p)
  {

  }
  HDF5Image::~HDF5Image()
  {

  }
  void HDF5Image::analyze(art::Event const & e)
  {

  }
  void HDF5Image::beginSubRun(art::SubRun const & sr)
  {

  }
  void HDF5Image::endSubRun(art::SubRun const & sr)
  {

  }

}