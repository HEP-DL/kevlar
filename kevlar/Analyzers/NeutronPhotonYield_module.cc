#include "kevlar/Analyzers/NeutronPhotonYield.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Principal/Handle.h"
#include "nusimdata/SimulationBase/MCParticle.h"

#include <iostream>
#include <map>

namespace kevlar{
  NeutronPhotonYield::NeutronPhotonYield(fhicl::ParameterSet const& pSet) :
    art::EDAnalyzer(pSet),
    fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
    fOutFileName(pSet.get<std::string>("OutFileName","NeutronPhotonYield.csv")),
    fCSVOut()
  {

  }
  NeutronPhotonYield::~NeutronPhotonYield()
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    }
  }
  void NeutronPhotonYield::analyze(art::Event const & evt)
  {
    art::Handle<std::vector<simb::MCParticle> > particles;
    evt.getByLabel(fProducerName, particles);
    std::map<int,int> neutronIds;
    for (auto particle: *particles){
      int pdg= particle.PdgCode();
      if (pdg != 2112) continue;
      neutronIds.insert(std::make_pair(particle.TrackId(),0 ));      
    }
    for (auto particle: *particles){
      int pdg = particle.PdgCode();
      if(pdg == 22 ){
        int mother = particle.Mother();
        if(neutronIds.find(mother) == neutronIds.end()) continue;
        neutronIds[mother] = neutronIds[mother]+1;
      }
    }
    for(std::map<int,int>::iterator it = neutronIds.begin(); 
        it != neutronIds.end(); ++it )
    {
      fCSVOut<<evt.id()<<" , "<<it->first<<" , "<<it->second<<std::endl;
    }
  }
  void NeutronPhotonYield::beginSubRun(art::SubRun const& )
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    }    
    fCSVOut.open(fOutFileName, 
      std::ofstream::out | std::ofstream::app);
    fCSVOut<<"EventID, TrackID, #Photons"<<std::endl;
  }
  void NeutronPhotonYield::endSubRun(art::SubRun const&)
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    } 
  }
}