#include "eventhsv2diagnosis.h"
#include "eventhsv2progression.h"
#include "eventhsv2transmission.h"
#include "eventprepoffered.h"
#include "person_hsv2.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventHSV2Diagnosis::EventHSV2Diagnosis(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventHSV2Diagnosis::~EventHSV2Diagnosis()
{
}

string EventHSV2Diagnosis::getDescription(double tNow) const
{
  return strprintf("HSV2 diagnosis event for %s", getPerson(0)->getName().c_str());
}

void EventHSV2Diagnosis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "HSV2 diagnosis", tNow, pPerson, 0);
}

void EventHSV2Diagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
{
  Person *pPerson = getPerson(0);
  
  // Infected partners (who possibly have a diagnosis event, of which
  // the hazard depends on the number of diagnosed partners), are also
  // affected!
  int numRel = pPerson->getNumberOfRelationships();
  
  pPerson->startRelationshipIteration();
  for (int i = 0 ; i < numRel ; i++)
  {
    double tDummy;
    Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);
    
    if (pPartner->hsv2().isInfected())
      population.markAffectedPerson(pPartner);
  }
  
#ifndef NDEBUG
  // Double check that the iteration is done
  double tDummy;
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

bool EventHSV2Diagnosis::isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);
  
  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTreatAcceptanceThreshold())
    return true;
  
  return false;  
}

bool EventHSV2Diagnosis::partnerWillingToTest(Person *pPerson, GslRandomNumberGenerator *pRndGen)
{
  // Person *pPerson = getPerson(0);
  
  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTestAcceptanceThreshold())
    return true;
  
  return false;
}

void EventHSV2Diagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  Person *pPerson = getPerson(0);
  
  assert(pPerson->hsv2().isInfected());
  
  // Mark person as diagnosed
  pPerson->hsv2().diagnose(t);
  double tLast = pPerson->getTimeLastSTI();
  pPerson->increaseSTIDiagnoseCount(t, tLast);
  
  // Update PrEP eligibility
  bool schedulePrePOfferedEvent = pPerson->hiv().updatePrePEligibility(t);
  // Check if  person has become eligible for PreP
  if (schedulePrePOfferedEvent) {
    // Schedule PreP being offered to this person
    EventPrePOffered *pEvtPreP = new EventPrePOffered(pPerson, true);
    population.onNewEvent(pEvtPreP);
  }
  
  // Start treatment?
  
  // Partner notification
  if(s_partnerNotificationEnabled){
    // Check relationships diagnosed person is in, and if partner is also infected, schedule diagnosis event if willing to get tested
    int numRelations = pPerson->getNumberOfRelationships();
    pPerson->startRelationshipIteration();
    
    for (int i = 0; i < numRelations; i++)
    {
      double formationTime = -1;
      Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
      
      if (pPartner->hsv2().isInfected() && !pPartner->hsv2().isDiagnosed())
      {
        if(partnerWillingToTest(pPartner, pRndGen)){
          
          EventHSV2Diagnosis *pEvtDiagP = new EventHSV2Diagnosis(pPartner, true);
          population.onNewEvent(pEvtDiagP);
          
        }
        
      }
    }
  }
  
}

bool EventHSV2Diagnosis::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if already diangosed or no longer infected
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->hsv2().isInfected() || pPerson1->hsv2().isDiagnosed()){
    return true;
  }
  
  return false;
}

double EventHSV2Diagnosis::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionHSV2Diagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  
  double internalTimeInterval = h.calculateInternalTimeInterval(t0, dt);

  return internalTimeInterval;
  // return h.calculateInternalTimeInterval(t0, dt);
}

double EventHSV2Diagnosis::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionHSV2Diagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  double realTimeInterval = h.solveForRealTimeInterval(t0, Tdiff);

  return realTimeInterval;
  
  // return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventHSV2Diagnosis::getTMax(const Person *pPerson)
{
  assert(pPerson != 0);
  double tb = pPerson->getDateOfBirth();
  
  assert(s_tMax > 0);
  return tb + s_tMax;
}

int EventHSV2Diagnosis::getD(Person *pPerson)
{
  // Person *pPerson = getPerson(0);
  
  int D1 = 0;
  int numRelations = pPerson->getNumberOfRelationships();
  double tDummy = 0;
  
  pPerson->startRelationshipIteration();
  for(int i = 0 ; i < numRelations; i++)
  {
    Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);
    if (pPartner->hsv2().isDiagnosed())
      D1++;
  }
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
  
  return D1;
}

double EventHSV2Diagnosis::s_baseline = 0;
double EventHSV2Diagnosis::s_diagPartnersFactor = 0;
double EventHSV2Diagnosis::s_healthSeekingPropensityFactor = 0;
double EventHSV2Diagnosis::s_beta = 0;
double EventHSV2Diagnosis::s_tMax = 0;

bool EventHSV2Diagnosis::s_partnerNotificationEnabled = false;

void EventHSV2Diagnosis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("hsv2.diagnosis.baseline", s_baseline)) ||
      !(r = config.getKeyValue("hsv2.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.getKeyValue("hsv2.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.getKeyValue("hsv2.diagnosis.beta", s_beta)) ||
      !(r = config.getKeyValue("hsv2.diagnosis.t_max", s_tMax)) ||
      !(r = config.getKeyValue("hsv2.diagnosis.partnernotification.enabled", s_partnerNotificationEnabled)) 
    )
    abortWithMessage(r.getErrorString());
}

void EventHSV2Diagnosis::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("hsv2.diagnosis.baseline", s_baseline)) ||
      !(r = config.addKey("hsv2.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.addKey("hsv2.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.addKey("hsv2.diagnosis.beta", s_beta)) ||
      !(r = config.addKey("hsv2.diagnosis.t_max", s_tMax)) ||
      !(r = config.addKey("hsv2.diagnosis.partnernotification.enabled", s_partnerNotificationEnabled))
  )
    abortWithMessage(r.getErrorString());
}

HazardFunctionHSV2Diagnosis::HazardFunctionHSV2Diagnosis(Person *pPerson, double baseline, double diagPartnersFactor, double healthSeekingPropensityFactor, double beta)
  : m_baseline(baseline), m_diagPartnersFactor(diagPartnersFactor), m_healthSeekingPropensityFactor(healthSeekingPropensityFactor), m_beta(beta)
{
  assert(pPerson != 0);
  m_pPerson = pPerson;
  
  double tinf = pPerson->hsv2().getInfectionTime();
  double H = pPerson->getHealthSeekingPropensity();
  int D = EventHSV2Diagnosis::getD(pPerson);
  
  double A = baseline + diagPartnersFactor*D + healthSeekingPropensityFactor*H - beta*tinf;
  double B = beta;
  
  setAB(A, B);
}

// This implementation is not necessary for running, it is provided for testing purposes
double HazardFunctionHSV2Diagnosis::evaluate(double t)
{
  double tinf = m_pPerson->hsv2().getInfectionTime();
  double H = m_pPerson->getHealthSeekingPropensity();
  int D = EventHSV2Diagnosis::getD(m_pPerson);
  
  return std::exp(m_baseline + m_diagPartnersFactor*D + m_healthSeekingPropensityFactor*H + m_beta*(t-tinf));
}

ConfigFunctions hsv2DiagnosisConfigFunctions(EventHSV2Diagnosis::processConfig, EventHSV2Diagnosis::obtainConfig, "EventHSV2Diagnosis");

JSONConfig hsv2DiagnosisJSONConfig(R"JSON(
    "EventHSV2Diagnosis": {
  "depends": null,
  "params": [
  ["hsv2.diagnosis.baseline", 0],
  ["hsv2.diagnosis.diagpartnersfactor", 0 ],
  ["hsv2.diagnosis.beta", 0],
  ["hsv2.diagnosis.healthseekingpropensityfactor", 0],
  ["hsv2.diagnosis.t_max", 200],
  ["hsv2.diagnosis.partnernotification.enabled", "no"]
  ],
            "info": [
  "TO DO"
            ]
})JSON");

