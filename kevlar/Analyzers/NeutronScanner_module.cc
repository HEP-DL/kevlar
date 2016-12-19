#include "kevlar/Analyzers/NeutronScanner.hh"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Principal/Handle.h"
#include "nusimdata/SimulationBase/MCParticle.h"

namespace kevlar{
  NeutronScanner::NeutronScanner(fhicl::ParameterSet const& pSet) :
    art::EDAnalyzer(pSet),
    fProducerName(pSet.get<std::string>("ProducerLabel","largeant")),
    fOutFileName(pSet.get<std::string>("OutFileName","neutronscanner.csv")),
    fCSVOut()
  {

  }
  NeutronScanner::~NeutronScanner()
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    }
  }
  void NeutronScanner::analyze(art::Event const & evt)
  {
    art::Handle<std::vector<simb::MCParticle> > particles;
    evt.getByLabel(fProducerName, particles);
    for (auto particle: *particles){
      int pdg= particle.PdgCode();
      //Cut on Neutrons
      if (pdg != 2112) continue;
      int mother = particle.Mother();
      std::string process = particle.Process();
      double x = particle.Position()[0];
      double y = particle.Position()[1];
      double z = particle.Position()[2];
      double t = particle.T();
      double px = particle.Px();
      double py = particle.Py();
      double pz = particle.Pz();
      double e = particle.E();
      fCSVOut<<mother<<" , "<<process<<" , "<<x<<" , "<<y<<" , "<<z<<" , "<<t<<" , "<<px<<" , "<<py<<" , "<<pz<<" , "<<e<<std::endl;
    }
  }
  void NeutronScanner::beginSubRun(art::SubRun const& )
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    }    
    fCSVOut.open(fOutFileName, 
      std::ofstream::out | std::ofstream::app);
    fCSVOut<<"Mother,process,x,y,z,t,Px,Py,Pz,E,";
  }
  void NeutronScanner::endSubRun(art::SubRun const&)
  {
    if(fCSVOut.is_open())
    {
      fCSVOut.close();
    } 
  }
}