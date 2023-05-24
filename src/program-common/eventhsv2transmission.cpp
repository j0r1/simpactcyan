#include "eventhsv2transmission.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "gslrandomnumbergenerator.h"
#include "util.h"
#include <cmath>
#include <iostream>

using namespace std;

// Conception happens between two people, so using this constructor seems natural.
// Also, when one of the involved persons dies before this is fired, the event is
// removed automatically.
EventHSV2Transmission::EventHSV2Transmission(Person *pPerson1, Person *pPerson2) : SimpactEvent(pPerson1, pPerson2)
{
  // is about transmission from pPerson1 to pPerson2, so no ordering according to
  // gender here
  assert(pPerson1->hsv2().isInfectious() && !pPerson2->hsv2().isInfected());
}

EventHSV2Transmission::~EventHSV2Transmission()
{
}

string EventHSV2Transmission::getDescription(double tNow) const
{
  return strprintf("HSV2 Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventHSV2Transmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  writeEventLogStart(true, "HSV2 transmission", tNow, pPerson1, pPerson2);
}

// The dissolution event that makes this event useless involves the exact same people,
// so this function will automatically make sure that this conception event is discarded
// (this function is definitely called for those people)
bool EventHSV2Transmission::isUseless(const PopulationStateInterface &population)
{
  // Transmission from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // If person2 already became HSV2 positive, there no sense in further transmission
  if (pPerson2->hsv2().isInfected())
    return true;
  
  if (!pPerson1->hsv2().isInfectious())
    return true;
  
  // If someone is in final AIDS stage
  if (pPerson1->hiv().getInfectionStage() == Person_HIV::AIDSFinal || pPerson2->hiv().getInfectionStage() == Person_HIV::AIDSFinal) {
    return true;
  }
  
  // Event is useless if the relationship between the two people is over
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

void EventHSV2Transmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
  assert(!pTarget->hsv2().isInfected());
  
  // Set Infection Type and Site (depending on sexual role)
  
  // For versatile men, sexual role depends on role of partner
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  
  int pRole1 = pTarget->getPreferredSexualRole();
  // if(pOrigin != 0) int pRole2 = pOrigin->getPreferredSexualRole();
  
  if(pTarget->isMan()){
    if(pOrigin == 0 && pRole1 == 0){ // for seeders
      double randnum = pRndGen->pickRandomDouble();
      if(randnum < 0.5){
        pRole1 = 1; // receptive
      }else{
        pRole1 = 2; // insertive
      }
    }
    else if(pOrigin != 0){
      if(pOrigin->isMan()){
        Person_HSV2::InfectionSite originSite = pOrigin->hsv2().getInfectionSite();
        if(originSite == Person_HSV2::Rectal){
          pRole1 = 2;
        }else if(originSite == Person_HSV2::Urethral){
          pRole1 = 1;
        }
      }
    }
  }
  
  if (pOrigin == 0){ 
    if(pTarget->isMan()){
      if(pRole1 == 1){
        pTarget->hsv2().setInfected(t, 0, Person_HSV2::Seed, Person_HSV2::Rectal);
      }else if(pRole1 == 2){
        pTarget->hsv2().setInfected(t, 0, Person_HSV2::Seed, Person_HSV2::Urethral);
      }
    }else if(pTarget->isWoman()){ // women
      pTarget->hsv2().setInfected(t, 0, Person_HSV2::Seed, Person_HSV2::Vaginal);
    }
  }else{
    assert(pOrigin->hsv2().isInfectious());
    
    if(pTarget->isMan() && pOrigin->isMan()){
      if(pRole1 == 1){
        pTarget->hsv2().setInfected(t, pOrigin, Person_HSV2::Partner, Person_HSV2::Rectal);
      }else if(pRole1 == 2){
        pTarget->hsv2().setInfected(t, pOrigin, Person_HSV2::Partner, Person_HSV2::Urethral);
      }
    }
    else if(pTarget->isMan() && pOrigin->isWoman()){ // heterosexual relationship
      pTarget->hsv2().setInfected(t, pOrigin, Person_HSV2::Partner, Person_HSV2::Urethral);
    }
    else if(pTarget->isWoman()){ // women
      pTarget->hsv2().setInfected(t, pOrigin, Person_HSV2::Partner, Person_HSV2::Vaginal);
    }
  }
  
  
  // Check relationships pTarget is in, and if the partner is not yet infected, schedule
  // a transmission event.
  int numRelations = pTarget->getNumberOfRelationships();
  pTarget->startRelationshipIteration();
  
  for (int i = 0 ; i < numRelations ; i++)
  {
    double formationTime = -1;
    Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);
    
    if (!pPartner->hsv2().isInfected())
    {
      EventHSV2Transmission *pEvtTrans = new EventHSV2Transmission(pTarget, pPartner);
      population.onNewEvent(pEvtTrans);
    }
  }
  
#ifndef NDEBUG
  double tDummy;
  assert(pTarget->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

void EventHSV2Transmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  // Transmission from pPerson1 to pPerson2
  Person *pPerson1 = getPerson(0);
  Person *pPerson2 = getPerson(1);
  
  // Person 1 should be infectious , person 2 should not be infected yet
  assert(pPerson1->hsv2().isInfectious());
  assert(!pPerson2->hsv2().isInfected());
  
  infectPerson(population, pPerson1, pPerson2, t);
}

double EventHSV2Transmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionHSV2Transmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventHSV2Transmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  Person *pOrigin = getPerson(0);
  Person *pTarget = getPerson(1);
  double tMax = getTMax(pOrigin, pTarget);
  
  HazardFunctionHSV2Transmission h0(pOrigin, pTarget, pState);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventHSV2Transmission::s_tMax = 200;
double EventHSV2Transmission::s_d1 = 0;
double EventHSV2Transmission::s_d2 = 0;
double EventHSV2Transmission::s_g1 = 0;
double EventHSV2Transmission::s_g2 = 0;
double EventHSV2Transmission::s_f = 0;
double EventHSV2Transmission::s_h = 0;
double EventHSV2Transmission::s_w = 0;
double EventHSV2Transmission::s_e1 = 0;
double EventHSV2Transmission::s_e2 = 0;
double EventHSV2Transmission::HazardFunctionHSV2Transmission::s_b = 0;

void EventHSV2Transmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("hsv2transmission.hazard.b", HazardFunctionHSV2Transmission::s_b)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.d1", s_d1)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.d2", s_d2)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.g1", s_g1)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.g2", s_g2)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.f", s_f)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.h", s_h)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.w", s_w)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.e1", s_e1)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.e2", s_e2)) ||
      !(r = config.getKeyValue("hsv2transmission.hazard.t_max", s_tMax))
  )
    abortWithMessage(r.getErrorString());
}

void EventHSV2Transmission::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("hsv2transmission.hazard.b", HazardFunctionHSV2Transmission::s_b)) ||
      !(r = config.addKey("hsv2transmission.hazard.d1", s_d1)) ||
      !(r = config.addKey("hsv2transmission.hazard.d2", s_d2)) ||
      !(r = config.addKey("hsv2transmission.hazard.g1", s_g1)) ||
      !(r = config.addKey("hsv2transmission.hazard.g2", s_g2)) ||
      !(r = config.addKey("hsv2transmission.hazard.f", s_f)) ||
      !(r = config.addKey("hsv2transmission.hazard.h", s_h)) ||
      !(r = config.addKey("hsv2transmission.hazard.w", s_w)) ||
      !(r = config.addKey("hsv2transmission.hazard.e1", s_e1))||
      !(r = config.addKey("hsv2transmission.hazard.e2", s_e2))||
      !(r = config.addKey("hsv2transmission.hazard.t_max", s_tMax))
  )
    abortWithMessage(r.getErrorString());
}

double calculateHazardFactor(const SimpactPopulation &population, double t0);

double EventHSV2Transmission::getTMax(const Person *pPerson1, const Person *pPerson2)
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
int EventHSV2Transmission::getH(const Person *pPerson1){
  assert(pPerson1 != 0);
  bool H1 = pPerson1->hiv().isInfected();
  int H = 0;
  if (H1 == true)
    H = 1;
  return H;
}

// get sexual role (infection site) of susceptible partner: TO DO (how to do without the randomness, get sexRole assigned per relationship?)
int EventHSV2Transmission::getR(const Person *pPerson1, const Person*pPerson2){
  assert(pPerson1 != 0);
  assert(pPerson2 != 0);
  
  // heterosexual
  int R = 0;
  
  if(pPerson1->isWoman()) // for women (always vaginal)
    R = 0;
  
  // MSM
  if(pPerson1->isMan() && pPerson2->isMan()){
    
    Person_HSV2::InfectionSite originSite = pPerson2->hsv2().getInfectionSite(); // infection site of origin
    if(originSite == Person_HSV2::Urethral)
      R = 1;
  }
  
  return R;
}

int EventHSV2Transmission::getW(const Person *pPerson1){
  assert(pPerson1 != 0);
  int W = 0;
  if(pPerson1->isWoman())
    W = 1;
  
  return W;
}


EventHSV2Transmission::HazardFunctionHSV2Transmission::HazardFunctionHSV2Transmission(const Person *pPerson1, 
                                                                                      const Person *pPerson2,
                                                                                      const State *pState)
  : HazardFunctionExp(getA(pPerson1, pPerson2, pState), s_b)
{
}

EventHSV2Transmission::HazardFunctionHSV2Transmission::~HazardFunctionHSV2Transmission()
{
}

double EventHSV2Transmission::HazardFunctionHSV2Transmission::getA(const Person *pOrigin, const Person *pTarget, const State *pState)
{
  assert(pOrigin);
  assert(pTarget);

  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  bool CondomUse = ((pOrigin->usesCondom(pTarget->hiv().isDiagnosed(), population.getRandomNumberGenerator())) ||
                    (pTarget->usesCondom(pOrigin->hiv().isDiagnosed(), population.getRandomNumberGenerator())));
  
  double Pi = pOrigin->getNumberOfRelationships();
  double Pj = pTarget->getNumberOfRelationships();
  
  return pOrigin->hsv2().getHazardAParameter() - s_b * pOrigin->hsv2().getInfectionTime() + 
    s_d1*EventHSV2Transmission::getH(pOrigin) + s_d2*EventHSV2Transmission::getH(pTarget) + 
    s_d1*Pi + s_d2*Pj +
    s_f*EventHSV2Transmission::getR(pTarget, pOrigin) + s_w*EventHSV2Transmission::getW(pTarget) +
    s_h*CondomUse + 
    s_e1*pTarget->hiv().getHazardB0Parameter() + s_e2*pTarget->hsv2().getHazardB2Parameter();
}

ConfigFunctions hsv2TransmissionConfigFunctions(EventHSV2Transmission::processConfig, EventHSV2Transmission::obtainConfig, 
                                                "EventHSV2Transmission");

JSONConfig hsv2TransmissionJSONConfig(R"JSON(
    "EventHSV2Transmission": { 
  "depends": null,
  "params": [ 
  [ "hsv2transmission.hazard.b", 0 ],
  [ "hsv2transmission.hazard.d1", 0 ],
  [ "hsv2transmission.hazard.d2", 0 ],
  [ "hsv2transmission.hazard.g1", 0 ],
  [ "hsv2transmission.hazard.g2", 0 ],
  [ "hsv2transmission.hazard.f", 0 ],
  [ "hsv2transmission.hazard.h", 0 ],
  [ "hsv2transmission.hazard.w", 0],
  [ "hsv2transmission.hazard.e1", 0 ],
  [ "hsv2transmission.hazard.e2", 0 ],
  [ "hsv2transmission.hazard.t_max", 200 ]
  ],
            "info": [ 
  "These configuration parameters allow you to set the 'b', 'c' and 'd' values in the hazard",
  " h = exp(a_i + b*(t-t_infected)+ c*M_i + d*H_i + e1*b0_j + e2*b2_j)",
  "The value of 'a_i' depends on the individual, and can be specified as a ",
  "distribution in the person parameters ",
  "The value of 'b0_j' depends on the individual, and can be specified as a ",
  "distribution in the person parameters ",
  "The value of 'b2_j' depends on the individual, and can be specified as a ",
  "distribution in the person parameters."
            ]
})JSON");

