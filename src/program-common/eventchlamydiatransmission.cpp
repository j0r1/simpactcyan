#include "eventchlamydiatransmission.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "eventchlamydiaprogression.h"
#include "eventchlamydiadiagnosis.h"
#include "gslrandomnumbergenerator.h"
#include "simpactpopulation.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventChlamydiaTransmission::EventChlamydiaTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
  assert(pPerson1->chlamydia().isInfectious() && !pPerson2->chlamydia().isInfected());
}

EventChlamydiaTransmission::~EventChlamydiaTransmission() {}

string EventChlamydiaTransmission::getDescription(double tNow) const
{
  return strprintf("Chlamydia Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventChlamydiaTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  writeEventLogStart(true, "Chlamydia transmission", tNow, pPerson1, pPerson2);
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or recovery / cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventChlamydiaTransmission::isUseless(const PopulationStateInterface &population)
{
  // Transmission happens from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // If person2 was already infected or is immune, there is no sense in further transmission
  if (pPerson2->chlamydia().isInfected()){ // || pPerson2->chlamydia().isImmune()) {
    return true;
  }
  // If person1 is no longer infectious, they can no longer transmit it
  if (!pPerson1->chlamydia().isInfectious()) {
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

void EventChlamydiaTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
  assert(!pTarget->chlamydia().isInfected());
  
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
        pTarget->chlamydia().setInfected(t, 0, Person_Chlamydia::Seed, Person_Chlamydia::Rectal);
      }else if(pRole1 == 2){
        pTarget->chlamydia().setInfected(t, 0, Person_Chlamydia::Seed, Person_Chlamydia::Urethral);
      }
    }else if(pTarget->isWoman()){ // women
      pTarget->chlamydia().setInfected(t, 0, Person_Chlamydia::Seed, Person_Chlamydia::Vaginal);
    }
  }else{
    assert(pOrigin->chlamydia().isInfectious());
    Person_Chlamydia::InfectionSite originSite = pOrigin->chlamydia().getInfectionSite();
    if(pTarget->isMan() && pOrigin->isMan()){ // MSM
      if(originSite == Person_Chlamydia::Urethral){
        pTarget->chlamydia().setInfected(t, pOrigin, Person_Chlamydia::Partner, Person_Chlamydia::Rectal);
      }else if(originSite == Person_Chlamydia::Rectal){
        pTarget->chlamydia().setInfected(t, pOrigin, Person_Chlamydia::Partner, Person_Chlamydia::Urethral);
      }
    }else if(pTarget->isMan() && pOrigin->isWoman()){ // heterosexual F->M
      pTarget->chlamydia().setInfected(t, pOrigin, Person_Chlamydia::Partner, Person_Chlamydia::Urethral);
    }else if(pTarget->isWoman()){ // heterosexual M->F
      pTarget->chlamydia().setInfected(t, pOrigin, Person_Chlamydia::Partner, Person_Chlamydia::Vaginal);
    }
  }
  
  pTarget->writeToChlamydiaLog();
  
  // Check relationships pTarget is in, and if partner not yet infected, schedule transmission event.
  int numRelations = pTarget->getNumberOfRelationships();
  pTarget->startRelationshipIteration();
  for (int i = 0; i < numRelations; i++)
  {
    double formationTime = -1;
    Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);
    
    if (!pPartner->chlamydia().isInfected())
    {
      EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pTarget, pPartner);
      population.onNewEvent(pEvtTrans);
    }
  }
  
  // Schedule progression event (= natural clearance) for newly infected person
  EventChlamydiaProgression *pEvtProgression = new EventChlamydiaProgression(pTarget);
  population.onNewEvent(pEvtProgression);
  
  // Schedule diagnosis event for symptomatic individuals
  int diseaseStage = pTarget->chlamydia().getDiseaseStage();
  if(diseaseStage == Person_Chlamydia::Symptomatic){
    EventChlamydiaDiagnosis *pEvtDiagnosis = new EventChlamydiaDiagnosis(pTarget, false);
    population.onNewEvent(pEvtDiagnosis);
  }
  
#ifndef NDEBUG
  double tDummy;
  assert(pTarget->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}


void EventChlamydiaTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  // Transmission from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // Person 1 should be infectious , person 2 should not be infected yet
  assert(pPerson1->chlamydia().isInfectious());
  assert(!pPerson2->chlamydia().isInfected());
  
  infectPerson(population, pPerson1, pPerson2, t);
}

double EventChlamydiaTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionChlamydiaTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventChlamydiaTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionChlamydiaTransmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventChlamydiaTransmission::s_a = 0;
double EventChlamydiaTransmission::s_tMax = 200;
double EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::s_b = 0;
double EventChlamydiaTransmission::s_d1 = 0;
double EventChlamydiaTransmission::s_d2 = 0;
double EventChlamydiaTransmission::s_e1 = 0;
double EventChlamydiaTransmission::s_e2 = 0;
double EventChlamydiaTransmission::s_f = 0;
double EventChlamydiaTransmission::s_h = 0;
double EventChlamydiaTransmission::s_w = 0;


void EventChlamydiaTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("chlamydiatransmission.hazard.a", s_a)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.b", HazardFunctionChlamydiaTransmission::s_b)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.t_max", s_tMax)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.d1", s_d1)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.d2", s_d2)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.e1", s_e1)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.e2", s_e2)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.f", s_f)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.h", s_h)) ||
      !(r = config.getKeyValue("chlamydiatransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

void EventChlamydiaTransmission::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("chlamydiatransmission.hazard.a", s_a))||
      !(r = config.addKey("chlamydiatransmission.hazard.b", HazardFunctionChlamydiaTransmission::s_b)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.t_max", s_tMax)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.d1", s_d1)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.d2", s_d2)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.e1", s_e1)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.e2", s_e2)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.f", s_f)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.h", s_h)) ||
      !(r = config.addKey("chlamydiatransmission.hazard.w", s_w))
  )
    abortWithMessage(r.getErrorString());
}

double calculateHazardFactor(const SimpactPopulation &population, double t0);


double EventChlamydiaTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
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
int EventChlamydiaTransmission::getH(const Person *pPerson1){
  assert(pPerson1 != 0);
  bool H1 = pPerson1->hiv().isInfected();
  int H = 0;
  if (H1 == true)
    H = 1;
  return H;
}

// get sexual role (infection site) of susceptible partner == infection site of infectious partner (pOrigin)
int EventChlamydiaTransmission::getR(const Person *pPerson1, const Person *pPerson2){
  assert(pPerson1 != 0);
  assert(pPerson2 != 0);
  
  int R = 0; // for women & insertive MSM
  Person_Chlamydia::InfectionSite originSite = pPerson2->chlamydia().getInfectionSite();
  if(!(pPerson1->isWoman()) && originSite == Person_Chlamydia::Urethral){ // if susceptible is not a woman and infectious partner is insertive
    R = 1;
  }
  
  return R;
}

int EventChlamydiaTransmission::getW(const Person *pPerson1){
  assert(pPerson1 != 0);
  int W = 0;
  if(pPerson1->isWoman())
    W = 1;
  
  return W;
}


EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::HazardFunctionChlamydiaTransmission(const Person *pPerson1,
                                                                                                     const Person *pPerson2,
                                                                                                     const State *pState)
  : HazardFunctionExp(getA(pPerson1, pPerson2, pState), s_b)
{
}

EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::~HazardFunctionChlamydiaTransmission()
{
}

double EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::getA(const Person *pOrigin, const Person *pTarget, const State *pState)
{
  assert(pOrigin);
  assert(pTarget);
  
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  bool CondomUse = ((pOrigin->usesCondom(pTarget->hiv().isDiagnosed(), population.getRandomNumberGenerator())) ||
                    (pTarget->usesCondom(pOrigin->hiv().isDiagnosed(), population.getRandomNumberGenerator())));
  
  double Pi = pOrigin->getNumberOfRelationships();
  double Pj = pTarget->getNumberOfRelationships();
  
  return s_a - s_b * pOrigin->chlamydia().getInfectionTime() + 
    s_d1*Pi + s_d2*Pj +
    s_e1*EventChlamydiaTransmission::getH(pOrigin) + s_e2*EventChlamydiaTransmission::getH(pTarget) + 
    s_f*EventChlamydiaTransmission::getR(pTarget, pOrigin) + s_w*EventChlamydiaTransmission::getW(pTarget) +
    s_h*CondomUse;
}

ConfigFunctions chlamydiaTransmissionConfigFunctions(EventChlamydiaTransmission::processConfig,
                                                     EventChlamydiaTransmission::obtainConfig,
                                                     "EventChlamydiaTransmission");

JSONConfig chlamydiaTransmissionJSONConfig(R"JSON(
    "EventChlamydiaTransmission": {
  "depends": null,
  "params": [
  [ "chlamydiatransmission.hazard.a", 0 ],
  [ "chlamydiatransmission.hazard.b", 0 ],
  [ "chlamydiatransmission.hazard.t_max", 200 ],
  [ "chlamydiatransmission.hazard.d1", 0],
  [ "chlamydiatransmission.hazard.d2", 0],
  [ "chlamydiatransmission.hazard.e1", 0 ],
  [ "chlamydiatransmission.hazard.e2", 0 ],
  [ "chlamydiatransmission.hazard.f", 0 ],
  [ "chlamydiatransmission.hazard.h", 0 ],
  [ "chlamydiatransmission.hazard.w", 0]
  ],
            "info": [
  "These configuration parameters allow you to set the 'a' and 'b' in the hazard",
  " h = exp(a + b*(t-t_infected)"
            ]
})JSON");
