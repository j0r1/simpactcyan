#include "eventgonorrheatransmission.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "eventgonorrheaprogression.h"
#include "eventgonorrheadiagnosis.h"
#include "gslrandomnumbergenerator.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventGonorrheaTransmission::EventGonorrheaTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
  assert(pPerson1->gonorrhea().isInfectious() && !pPerson2->gonorrhea().isInfected());
}


EventGonorrheaTransmission::~EventGonorrheaTransmission()
{
}

string EventGonorrheaTransmission::getDescription(double tNow) const
{
  return strprintf("Gonorrhea Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventGonorrheaTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  writeEventLogStart(true, "Gonorrhea transmission", tNow, pPerson1, pPerson2);
  
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or recovery / cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventGonorrheaTransmission::isUseless(const PopulationStateInterface &population)
{
  // Transmission happens from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // If person2 was already infected with gonorrhea, there is no sense in further transmission
  if (pPerson2->gonorrhea().isInfected()) {
    return true;
  }
  // If person1 is no longer infectious, they can no longer transmit it
  if (!pPerson1->gonorrhea().isInfectious()) {
    return true;
  }
  
  // If someone is in final AIDS stage
  if (pPerson1->hiv().getInfectionStage() == Person_HIV::AIDSFinal || pPerson2->hiv().getInfectionStage() == Person_HIV::AIDSFinal) {
    return true;
  }
  
  // If relationship is over, transmission event is useless
  if (!pPerson1->hasRelationshipWith(pPerson2))
  {
    assert(!pPerson2->hasRelationshipWith(pPerson1));
    return true;
  }
  
  // Make sure the two lists are consistent: if person1 has a relationship with person2, person2
  // should also have a relationship with person1
  assert(pPerson2->hasRelationshipWith(pPerson1));
  
  return false;
}

void EventGonorrheaTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
  // check that person is not yet infected
  assert(!pTarget->gonorrhea().isInfected());
  
  // Set Infection Type and Site (depending on sexual role)
  
  // For versatile men, sexual role depends on role of partner
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  
  int pRole1 = pTarget->getPreferredSexualRole();

  if(pTarget->isMan()){
    if(pOrigin == 0 && pRole1 == 0){ // for seeders
      double randnum = pRndGen->pickRandomDouble();
      if(randnum < 0.5){
        pRole1 = 1; // receptive
      }else{
        pRole1 = 2; // insertive
      }
    }
  }
  
  if (pOrigin == 0){ 
    if(pTarget->isMan()){
      if(pRole1 == 1){
        pTarget->gonorrhea().setInfected(t, 0, Person_Gonorrhea::Seed, Person_Gonorrhea::Rectal);
      }else if(pRole1 == 2){
        pTarget->gonorrhea().setInfected(t, 0, Person_Gonorrhea::Seed, Person_Gonorrhea::Urethral);
      }
    }else if(pTarget->isWoman()){ // women
      pTarget->gonorrhea().setInfected(t, 0, Person_Gonorrhea::Seed, Person_Gonorrhea::Vaginal);
    }
  }else{
    assert(pOrigin->gonorrhea().isInfectious());
    Person_Gonorrhea::InfectionSite originSite = pOrigin->gonorrhea().getInfectionSite();
    if(pTarget->isMan() && pOrigin->isMan()){ // MSM
      if(originSite == Person_Gonorrhea::Urethral){
        pTarget->gonorrhea().setInfected(t, pOrigin, Person_Gonorrhea::Partner, Person_Gonorrhea::Rectal);
      }else if(originSite == Person_Gonorrhea::Rectal){
        pTarget->gonorrhea().setInfected(t, pOrigin, Person_Gonorrhea::Partner, Person_Gonorrhea::Urethral);
      }
    }else if(pTarget->isMan() && pOrigin->isWoman()){ // heterosexual F->M
      pTarget->gonorrhea().setInfected(t, pOrigin, Person_Gonorrhea::Partner, Person_Gonorrhea::Urethral);
    }else if(pTarget->isWoman()){ // heterosexual M->F
      pTarget->gonorrhea().setInfected(t, pOrigin, Person_Gonorrhea::Partner, Person_Gonorrhea::Vaginal);
    }
  }
  
  pTarget->writeToGonorrheaLog();
  
  // Check relationships pTarget is in, and if partner not yet infected with gonorrhea, schedule transmission event.
  int numRelations = pTarget->getNumberOfRelationships();
  pTarget->startRelationshipIteration();
  
  for (int i = 0; i < numRelations; i++)
  {
    double formationTime = -1;
    Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);
    
    if (!pPartner->gonorrhea().isInfected())
    {
      EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pTarget, pPartner);
      population.onNewEvent(pEvtTrans);
    }
  }
  
  // Schedule progression event (= natural clearance) for newly infected person
  EventGonorrheaProgression *pEvtProgression = new EventGonorrheaProgression(pTarget);
  population.onNewEvent(pEvtProgression);
  
  // Schedule diagnosis event for symptomatic individuals
  int diseaseStage = pTarget->gonorrhea().getDiseaseStage();
  if(diseaseStage == Person_Gonorrhea::Symptomatic){
    EventGonorrheaDiagnosis *pEvtDiagnosis = new EventGonorrheaDiagnosis(pTarget, false);
    population.onNewEvent(pEvtDiagnosis); 
  }
  
#ifndef NDEBUG
  double tDummy;
  assert(pTarget->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
  
}

void EventGonorrheaTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  // Transmission from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // Person 1 should be infectious , person 2 should not be infected yet
  assert(pPerson1->gonorrhea().isInfectious());
  assert(!pPerson2->gonorrhea().isInfected());
  
  infectPerson(population, pPerson1, pPerson2, t);
  
}

double EventGonorrheaTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionGonorrheaTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventGonorrheaTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionGonorrheaTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventGonorrheaTransmission::s_a = 0;
double EventGonorrheaTransmission::s_tMax = 200;
double EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::s_b = 0;
double EventGonorrheaTransmission::s_d1 = 0;
double EventGonorrheaTransmission::s_d2 = 0;
double EventGonorrheaTransmission::s_e1 = 0;
double EventGonorrheaTransmission::s_e2 = 0;
double EventGonorrheaTransmission::s_f = 0;
double EventGonorrheaTransmission::s_h = 0;
double EventGonorrheaTransmission::s_w = 0;

void EventGonorrheaTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("gonorrheatransmission.hazard.a", s_a)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.b", HazardFunctionGonorrheaTransmission::s_b)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.t_max", s_tMax)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.d1", s_d1)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.d2", s_d2)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.e1", s_e1)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.e2", s_e2)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.f", s_f)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.h", s_h)) ||
      !(r = config.getKeyValue("gonorrheatransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

void EventGonorrheaTransmission::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("gonorrheatransmission.hazard.a", s_a))||
      !(r = config.addKey("gonorrheatransmission.hazard.b", HazardFunctionGonorrheaTransmission::s_b)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.t_max", s_tMax)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.d1", s_d1)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.d2", s_d2)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.e1", s_e1)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.e2", s_e2)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.f", s_f)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.h", s_h)) ||
      !(r = config.addKey("gonorrheatransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

double calculateHazardFactor(const SimpactPopulation &population, double t0);

double EventGonorrheaTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
{
  assert(pPerson1 != 0 && pPerson2 != 0);
  
  double tb1 = pPerson1->getDateOfBirth();
  double tb2 = pPerson2->getDateOfBirth();
  
  double tMax = tb1;
  
  if (tb2 < tMax)
    tMax = tb2;
  
  assert(s_tMax > 0);
  tMax += s_tMax;
  return tMax;
}

// get HIV status 
int EventGonorrheaTransmission::getH(const Person *pPerson1){
  assert(pPerson1 != 0);
  bool H1 = pPerson1->hiv().isInfected();
  int H = 0;
  if (H1 == true)
    H = 1;
  return H;
}

// get sexual role (infection site) of susceptible partner: TO DO (how to do without the randomness, get sexRole assigned per relationship?)
int EventGonorrheaTransmission::getR(const Person *pPerson1, const Person*pPerson2){
  assert(pPerson1 != 0);
  assert(pPerson2 != 0);
  
  int R = 0; // for women & insertive MSM
  Person_Gonorrhea::InfectionSite originSite = pPerson2->gonorrhea().getInfectionSite();
  if(!(pPerson1->isWoman()) && originSite == Person_Gonorrhea::Urethral){ // if susceptible is not a woman and infectious partner is insertive
    R = 1;
  }
  
  return R;
}

int EventGonorrheaTransmission::getW(const Person *pPerson1){
  assert(pPerson1 != 0);
  int W = 0;
  if(pPerson1->isWoman())
    W = 1;
  
  return W;
}

EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::HazardFunctionGonorrheaTransmission(const Person *pPerson1,
                                                                                                     const Person *pPerson2, 
                                                                                                     const State *pState)
  : HazardFunctionExp(getA(pPerson1, pPerson2, pState), s_b)
{
}

EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::~HazardFunctionGonorrheaTransmission()
{
}

double EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::getA(const Person *pOrigin, const Person *pTarget, const State *pState)
{
  assert(pOrigin);
  assert(pTarget);
  
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  bool CondomUse = ((pOrigin->usesCondom(pTarget->hiv().isDiagnosed(), population.getRandomNumberGenerator())) ||
    (pTarget->usesCondom(pOrigin->hiv().isDiagnosed(), population.getRandomNumberGenerator())));
  
  double Pi = pOrigin->getNumberOfRelationships();
  double Pj = pTarget->getNumberOfRelationships();
  
  return s_a - s_b*pOrigin->gonorrhea().getInfectionTime() + 
    s_d1*Pi + s_d2*Pj +
    s_e1*EventGonorrheaTransmission::getH(pOrigin) + s_e2*EventGonorrheaTransmission::getH(pTarget) + 
    s_f*EventGonorrheaTransmission::getR(pTarget, pOrigin) + s_w*EventGonorrheaTransmission::getW(pTarget) +
    s_h*CondomUse;
}

ConfigFunctions gonorrheaTransmissionConfigFunctions(EventGonorrheaTransmission::processConfig,
                                                     EventGonorrheaTransmission::obtainConfig,
                                                     "EventGonorrheaTransmission");

JSONConfig gonorrheaTransmissionJSONConfig(R"JSON(
    "EventGonorrheaTransmission": {
  "depends": null,
  "params": [
  [ "gonorrheatransmission.hazard.a", 0 ],
  [ "gonorrheatransmission.hazard.b", 0 ],
  [ "gonorrheatransmission.hazard.t_max", 200 ],
  [ "gonorrheatransmission.hazard.d1", 0],
  [ "gonorrheatransmission.hazard.d2", 0],
  [ "gonorrheatransmission.hazard.e1", 0 ],
  [ "gonorrheatransmission.hazard.e2", 0 ],
  [ "gonorrheatransmission.hazard.f", 0 ],
  [ "gonorrheatransmission.hazard.h", 0 ],
  [ "gonorrheatransmission.hazard.w", 0]
  
  ],
            "info": [
  "These configuration parameters allow you to set the 'a' and 'b' in the hazard",
  " h = exp(a + b*(t-t_infected)"
            ]
})JSON");