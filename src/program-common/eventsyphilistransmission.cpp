#include "eventsyphilistransmission.h"
#include "gslrandomnumbergenerator.h"
#include "configfunctions.h"
#include "eventsyphilisprogression.h"
#include "jsonconfig.h"
#include "eventsyphilisdiagnosis.h"
#include "eventtertiarysyphilis.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventSyphilisTransmission::EventSyphilisTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
  assert(pPerson1->syphilis().isInfectious() && !pPerson2->syphilis().isInfected());
}

EventSyphilisTransmission::~EventSyphilisTransmission() {}

string EventSyphilisTransmission::getDescription(double tNow) const
{
  return strprintf("Syphilis Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventSyphilisTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  writeEventLogStart(true, "Syphilis transmission", tNow, pPerson1, pPerson2);
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or recovery / cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventSyphilisTransmission::isUseless(const PopulationStateInterface &population)
{
  // Transmission happens from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // If person2 is already infected, there is no sense in further transmission
  if (pPerson2->syphilis().isInfected()) {
    return true;
  }
  // If person1 is no longer infectious, they can no longer transmit it
  if (!pPerson1->syphilis().isInfectious()) {
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

void EventSyphilisTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
  assert(!pTarget->syphilis().isInfected());
  
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
        pTarget->syphilis().setInfected(t, 0, Person_Syphilis::Seed, Person_Syphilis::Rectal);
      }else if(pRole1 == 2){
        pTarget->syphilis().setInfected(t, 0, Person_Syphilis::Seed, Person_Syphilis::Urethral);
      }
    }else if(pTarget->isWoman()){ // women
      pTarget->syphilis().setInfected(t, 0, Person_Syphilis::Seed, Person_Syphilis::Vaginal);
    }
  }else{
    assert(pOrigin->syphilis().isInfectious());
    Person_Syphilis::InfectionSite originSite = pOrigin->syphilis().getInfectionSite();
    if(pTarget->isMan() && pOrigin->isMan()){ // MSM
      if(originSite == Person_Syphilis::Urethral){
        pTarget->syphilis().setInfected(t, pOrigin, Person_Syphilis::Partner, Person_Syphilis::Rectal);
      }else if(originSite == Person_Syphilis::Rectal){
        pTarget->syphilis().setInfected(t, pOrigin, Person_Syphilis::Partner, Person_Syphilis::Urethral);
      }
    }else if(pTarget->isMan() && pOrigin->isWoman()){ // heterosexual F->M
      pTarget->syphilis().setInfected(t, pOrigin, Person_Syphilis::Partner, Person_Syphilis::Urethral);
    }else if(pTarget->isWoman()){ // heterosexual M->F
      pTarget->syphilis().setInfected(t, pOrigin, Person_Syphilis::Partner, Person_Syphilis::Vaginal);
    }
  }
  
  pTarget->writeToSyphilisLog();
  
  // Check relationships pTarget is in, and if partner not yet infected with syphilis, schedule transmission event.
  int numRelations = pTarget->getNumberOfRelationships();
  pTarget->startRelationshipIteration();
  
  for (int i = 0; i < numRelations; i++)
  {
    double formationTime = -1;
    Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);
    
    if (!pPartner->syphilis().isInfected())
    {
      EventSyphilisTransmission *pEvtTrans = new EventSyphilisTransmission(pTarget, pPartner);
      population.onNewEvent(pEvtTrans);
    }
  }
  
  // Schedule progression event for newly infected person
  EventSyphilisProgression *pEvtProgression = new EventSyphilisProgression(pTarget);
  population.onNewEvent(pEvtProgression);
  
  // Schedule tertiary stage event
  EventTertiarySyphilis *pEvtTertiary = new EventTertiarySyphilis(pTarget);
  population.onNewEvent(pEvtTertiary);
  
  // Schedule diagnosis event 
  EventSyphilisDiagnosis *pEvtDiagnosis = new EventSyphilisDiagnosis(pTarget, false);
  population.onNewEvent(pEvtDiagnosis);
  
#ifndef NDEBUG
  double tDummy;
  assert(pTarget->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}


void EventSyphilisTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  // Transmission from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // Person 1 should be infectious , person 2 should not be infected yet
  assert(pPerson1->syphilis().isInfectious());
  assert(!pPerson2->syphilis().isInfected());
  
  infectPerson(population, pPerson1, pPerson2, t);
}

double EventSyphilisTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionSyphilisTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventSyphilisTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionSyphilisTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventSyphilisTransmission::s_a = 0;
double EventSyphilisTransmission::s_tMax = 200;
double EventSyphilisTransmission::HazardFunctionSyphilisTransmission::s_b = 0;
double EventSyphilisTransmission::s_d1 = 0;
double EventSyphilisTransmission::s_d2 = 0;
double EventSyphilisTransmission::s_e1 = 0;
double EventSyphilisTransmission::s_e2 = 0;
double EventSyphilisTransmission::s_f = 0;
double EventSyphilisTransmission::s_h = 0;
double EventSyphilisTransmission::s_w = 0;

void EventSyphilisTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("syphilistransmission.hazard.a", s_a)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.b", HazardFunctionSyphilisTransmission::s_b)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.t_max", s_tMax)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.d1", s_d1)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.d2", s_d2)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.e1", s_e1)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.e2", s_e2)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.f", s_f)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.h", s_h)) ||
      !(r = config.getKeyValue("syphilistransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

void EventSyphilisTransmission::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("syphilistransmission.hazard.a", s_a))||
      !(r = config.addKey("syphilistransmission.hazard.b", HazardFunctionSyphilisTransmission::s_b)) ||
      !(r = config.addKey("syphilistransmission.hazard.t_max", s_tMax)) ||
      !(r = config.addKey("syphilistransmission.hazard.d1", s_d1)) ||
      !(r = config.addKey("syphilistransmission.hazard.d2", s_d2)) ||
      !(r = config.addKey("syphilistransmission.hazard.e1", s_e1)) ||
      !(r = config.addKey("syphilistransmission.hazard.e2", s_e2)) ||
      !(r = config.addKey("syphilistransmission.hazard.f", s_f)) ||
      !(r = config.addKey("syphilistransmission.hazard.h", s_h)) ||
      !(r = config.addKey("syphilistransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

double calculateHazardFactor(const SimpactPopulation &population, double t0);

double EventSyphilisTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
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
int EventSyphilisTransmission::getH(const Person *pPerson1){
  assert(pPerson1 != 0);
  bool H1 = pPerson1->hiv().isInfected();
  int H = 0;
  if (H1 == true)
    H = 1;
  return H;
}

// get sexual role (infection site) of susceptible partner: TO DO (how to do without the randomness, get sexRole assigned per relationship?)
int EventSyphilisTransmission::getR(const Person *pPerson1, const Person*pPerson2){
  assert(pPerson1 != 0);
  assert(pPerson2 != 0);
  
  int R = 0; // for women & insertive MSM
  Person_Syphilis::InfectionSite originSite = pPerson2->syphilis().getInfectionSite();
  if(!(pPerson1->isWoman()) && originSite == Person_Syphilis::Urethral){ // if susceptible is not a woman and infectious partner is insertive
    R = 1;
  }
  
  return R;
}

int EventSyphilisTransmission::getW(const Person *pPerson1){
  assert(pPerson1 != 0);
  int W = 0;
  if(pPerson1->isWoman())
    W = 1;
  
  return W;
}


EventSyphilisTransmission::HazardFunctionSyphilisTransmission::HazardFunctionSyphilisTransmission(const Person *pPerson1,
                                                                                                  const Person *pPerson2,
                                                                                                  const State *pState)
  : HazardFunctionExp(getA(pPerson1, pPerson2, pState), s_b)
{}

EventSyphilisTransmission::HazardFunctionSyphilisTransmission::~HazardFunctionSyphilisTransmission() {}

double EventSyphilisTransmission::HazardFunctionSyphilisTransmission::getA(const Person *pOrigin, const Person *pTarget, const State *pState)
{
  assert(pOrigin);
  assert(pTarget);
  
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  bool CondomUse = ((pOrigin->usesCondom(pTarget->hiv().isDiagnosed(), population.getRandomNumberGenerator())) ||
                    (pTarget->usesCondom(pOrigin->hiv().isDiagnosed(), population.getRandomNumberGenerator())));
  
  double Pi = pOrigin->getNumberOfRelationships();
  double Pj = pTarget->getNumberOfRelationships();
  
  return s_a - s_b * pOrigin->syphilis().getInfectionTime() + 
    s_d1*Pi + s_d2*Pj +
    s_e1*EventSyphilisTransmission::getH(pOrigin) + s_e2*EventSyphilisTransmission::getH(pTarget) + 
    s_f*EventSyphilisTransmission::getR(pTarget, pOrigin) + s_w*EventSyphilisTransmission::getW(pTarget) +
    s_h*CondomUse;
}

ConfigFunctions syphilisTransmissionConfigFunctions(EventSyphilisTransmission::processConfig,
                                                    EventSyphilisTransmission::obtainConfig,
                                                    "EventSyphilisTransmission");

JSONConfig syphilisTransmissionJSONConfig(R"JSON(
    "EventSyphilisTransmission": {
  "depends": null,
  "params": [
  [ "syphilistransmission.hazard.a", 0 ],
  [ "syphilistransmission.hazard.b", 0 ],
  [ "syphilistransmission.hazard.t_max", 200 ],
  [ "syphilistransmission.hazard.d1", 0],
  [ "syphilistransmission.hazard.d2", 0],
  [ "syphilistransmission.hazard.e1", 0 ],
  [ "syphilistransmission.hazard.e2", 0 ],
  [ "syphilistransmission.hazard.f", 0 ],
  [ "syphilistransmission.hazard.h", 0 ],
  [ "syphilistransmission.hazard.w", 0]
  ],
            "info": [
  "These configuration parameters allow you to set the 'a' and 'b' in the hazard",
  " h = exp(a + b*(t-t_infected)"
            ]
})JSON");
