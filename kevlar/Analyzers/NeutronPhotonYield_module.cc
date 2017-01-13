#include "kevlar/Analyzers/NeutronPhotonYield.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Principal/Handle.h"
#include "nusimdata/SimulationBase/MCParticle.h"

#include <iostream>
#include <map>
#include <string>

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
    if(fCSVOut.is_open()) fCSVOut.close();
  }
  void NeutronPhotonYield::analyze(art::Event const & evt)
  {
    art::Handle<std::vector<simb::MCParticle> > particles;
    evt.getByLabel(fProducerName, particles);
    std::map<int,int> neutronIds;
    std::map<int, std::string> processes;
    std::map<int, double> energies;
    for (auto particle: *particles){
      if (particle.PdgCode() != 2112) continue;
      neutronIds.insert(std::make_pair(particle.TrackId(),0 ));
      energies[particle.TrackId()] = particle.E();
    }
    for (auto particle: *particles){
      if(particle.PdgCode()== 22 ){
        int mother = particle.Mother();
        if(neutronIds.find(mother) == neutronIds.end()) continue;
        neutronIds[mother] = neutronIds[mother]+1;
        processes[mother] = particle.Process();
      }
    }
    for(std::map<int,int>::iterator it = neutronIds.begin(); 
        it != neutronIds.end(); ++it )
      fCSVOut<<evt.id()<<", "<<it->first<<", "<<energies[it->first]<<", "
        <<it->second<<", "<<processes[it->first]<<std::endl;
  }
  void NeutronPhotonYield::beginSubRun(art::SubRun const& )
  {
    if(fCSVOut.is_open()) fCSVOut.close();
    fCSVOut.open(fOutFileName, 
      std::ofstream::out | std::ofstream::app);
    fCSVOut<<"EventID, TrackID, Energy, #Photons, Process"<<std::endl;
  }
  void NeutronPhotonYield::endSubRun(art::SubRun const&)
  {
    if(fCSVOut.is_open()) fCSVOut.close();
  }
}