#include "eventsyphilisdiagnosis.h"
#include "eventsyphilisprogression.h"
#include "eventsyphilistransmission.h"
#include "person_syphilis.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventSyphilisDiagnosis::EventSyphilisDiagnosis(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventSyphilisDiagnosis::~EventSyphilisDiagnosis()
{
}

string EventSyphilisDiagnosis::getDescription(double tNow) const
{
  return strprintf("Syphilis diagnosis event for %s", getPerson(0)->getName().c_str());
}

void EventSyphilisDiagnosis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "syphilis diagnosis", tNow, pPerson, 0);
}

void EventSyphilisDiagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
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
    
    if (pPartner->syphilis().isInfected())
      population.markAffectedPerson(pPartner);
  }
  
#ifndef NDEBUG
  // Double check that the iteration is done
  double tDummy;
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

bool EventSyphilisDiagnosis::isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);
  
  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTreatAcceptanceThreshold())
    return true;
  
  return false;  
}

void EventSyphilisDiagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  Person *pPerson = getPerson(0);
  
  assert(pPerson->syphilis().isInfected());
  
  // Mark person as diagnosed
  pPerson->syphilis().diagnose(t);
  double tLast = pPerson->getTimeLastSTI();
  pPerson->increaseSTIDiagnoseCount(t, tLast);
  
  // If accepting treatment --> immediate recovery
  if(isWillingToTreatSTI(t, pRndGen))
  {
    pPerson->syphilis().progress(t, true);
    pPerson->writeToSyphilisTreatLog();
    writeEventLogStart(true, "(syphilis treatment)", t, pPerson, 0);
    
    // Schedule new transmission event from infectious partners if person becomes susceptible again --> MOVE TO PERSON_CHLAMYDIA??
    Person_Syphilis::SyphilisDiseaseStage diseaseStage = pPerson->syphilis().getDiseaseStage();
    if (diseaseStage == Person_Syphilis::Susceptible) {
      int numRelations = pPerson->getNumberOfRelationships();
      pPerson->startRelationshipIteration();
      
      for (int i = 0; i < numRelations; i++)
      {
        double formationTime = -1;
        Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
        if (pPartner->syphilis().isInfectious())
        {
          EventSyphilisTransmission *pEvtTrans = new EventSyphilisTransmission(pPartner, pPerson);
          population.onNewEvent(pEvtTrans);
        }
      }
    }
  }
  
}

bool EventSyphilisDiagnosis::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if death (ie no longer infected) occurs before scheduled diagnosis
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->syphilis().isInfected()){
    return true;
  }
  
  return false;
}


double EventSyphilisDiagnosis::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionSyphilisDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventSyphilisDiagnosis::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionSyphilisDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventSyphilisDiagnosis::getTMax(const Person *pPerson)
{
  assert(pPerson != 0);
  double tb = pPerson->getDateOfBirth();
  
  assert(s_tMax > 0);
  return tb + s_tMax;
}

int EventSyphilisDiagnosis::getD(Person *pPerson)
{
  // Person *pPerson = getPerson(0);
  
  int D1 = 0;
  int numRelations = pPerson->getNumberOfRelationships();
  double tDummy = 0;
  
  pPerson->startRelationshipIteration();
  for(int i = 0 ; i < numRelations; i++)
  {
    Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);
    if (pPartner->syphilis().isDiagnosed())
      D1++;
  }
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
  
  return D1;
}

double EventSyphilisDiagnosis::s_baseline = 0;
double EventSyphilisDiagnosis::s_beta = 0;
double EventSyphilisDiagnosis::s_diagPartnersFactor = 0;
double EventSyphilisDiagnosis::s_healthSeekingPropensityFactor = 0;
double EventSyphilisDiagnosis::s_tMax = 0;

void EventSyphilisDiagnosis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("syphilis.diagnosis.baseline", s_baseline)) ||
      !(r = config.getKeyValue("syphilis.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.getKeyValue("syphilis.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.getKeyValue("syphilis.diagnosis.beta", s_beta)) ||
      !(r = config.getKeyValue("syphilis.diagnosis.t_max", s_tMax))
  )
    abortWithMessage(r.getErrorString());
}

void EventSyphilisDiagnosis::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("syphilis.diagnosis.baseline", s_baseline)) ||
      !(r = config.addKey("syphilis.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.addKey("syphilis.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.addKey("syphilis.diagnosis.beta", s_beta)) ||
      !(r = config.addKey("syphilis.diagnosis.t_max", s_tMax)) 
  )
    abortWithMessage(r.getErrorString());
}

HazardFunctionSyphilisDiagnosis::HazardFunctionSyphilisDiagnosis(Person *pPerson, double baseline, double diagPartnersFactor, double healthSeekingPropensityFactor, double beta)
  : m_baseline(baseline), m_diagPartnersFactor(diagPartnersFactor), m_healthSeekingPropensityFactor(healthSeekingPropensityFactor), m_beta(beta)
{
  assert(pPerson != 0);
  m_pPerson = pPerson;
  
  double tinf = pPerson->syphilis().getInfectionTime();
  double H = pPerson->getHealthSeekingPropensity();
  int D = EventSyphilisDiagnosis::getD(pPerson);
  
  double A = baseline + diagPartnersFactor*D + healthSeekingPropensityFactor*H - beta*tinf;
  double B = beta;
  
  setAB(A, B);
}

// This implementation is not necessary for running, it is provided for testing purposes
double HazardFunctionSyphilisDiagnosis::evaluate(double t)
{
  double tinf = m_pPerson->syphilis().getInfectionTime();
  double H = m_pPerson->getHealthSeekingPropensity();
  double D = EventSyphilisDiagnosis::getD(m_pPerson);
  
  return std::exp(m_baseline + m_diagPartnersFactor*D + m_healthSeekingPropensityFactor*H + m_beta*(t-tinf));
}

ConfigFunctions syphilisDiagnosisConfigFunctions(EventSyphilisDiagnosis::processConfig, EventSyphilisDiagnosis::obtainConfig, "EventSyphilisDiagnosis");

JSONConfig syphilisDiagnosisJSONConfig(R"JSON(
    "EventSyphilisDiagnosis": {
  "depends": null,
  "params": [
  ["syphilis.diagnosis.baseline", 0],
  ["syphilis.diagnosis.diagpartnersfactor", 0 ],
  ["syphilis.diagnosis.beta", 0],
  ["syphilis.diagnosis.healthseekingpropensityfactor", 0],
  ["syphilis.diagnosis.t_max", 200]
  ],
            "info": [
  "TO DO"
            ]
})JSON");
