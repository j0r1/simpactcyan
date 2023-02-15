#include "eventchlamydiadiagnosis.h"
#include "eventchlamydiaprogression.h"
#include "eventchlamydiatransmission.h"
#include "person_chlamydia.h"
#include "person.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventChlamydiaDiagnosis::EventChlamydiaDiagnosis(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventChlamydiaDiagnosis::~EventChlamydiaDiagnosis()
{
}

string EventChlamydiaDiagnosis::getDescription(double tNow) const
{
  return strprintf("Chlamydia diagnosis event for %s", getPerson(0)->getName().c_str());
}

void EventChlamydiaDiagnosis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "chlamydia diagnosis", tNow, pPerson, 0);
}

void EventChlamydiaDiagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
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
    
    if (pPartner->chlamydia().isInfected())
      population.markAffectedPerson(pPartner);
  }
  
#ifndef NDEBUG
  // Double check that the iteration is done
  double tDummy;
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

bool EventChlamydiaDiagnosis::isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);
  
  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTreatAcceptanceThreshold())
    return true;
  
  return false;  
}

void EventChlamydiaDiagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  Person *pPerson = getPerson(0);
  
  assert(pPerson->chlamydia().isInfected());
  
  // Mark person as diagnosed
  pPerson->chlamydia().diagnose(t);
  // pPerson->writeToChlamydiaTreatLog();
  double tLast = pPerson->getTimeLastSTI();
  pPerson->increaseSTIDiagnoseCount(t, tLast);
  
  // If accepting treatment --> immediate recovery
  if(isWillingToTreatSTI(t, pRndGen))
  {
    pPerson->chlamydia().progress(t, true); // treated 
    pPerson->writeToChlamydiaTreatLog();
    writeEventLogStart(true, "(chlamydia treatment)", t, pPerson, 0);
    
    // // Schedule new transmission event from infectious partners if person becomes susceptible again --> MOVE TO PERSON_CHLAMYDIA??
    Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
    if (diseaseStage == Person_Chlamydia::Susceptible) {
      int numRelations = pPerson->getNumberOfRelationships();
      pPerson->startRelationshipIteration();
      
      for (int i = 0; i < numRelations; i++)
      {
        double formationTime = -1;
        Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
        if (pPartner->chlamydia().isInfectious())
        {
          EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pPartner, pPerson);
          population.onNewEvent(pEvtTrans);
        }
      }
    }
    
  }
  
}

bool EventChlamydiaDiagnosis::isUseless(const PopulationStateInterface &population)
{
  // Event becomes useless if natural clearance occurs before scheduled diagnosis
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->chlamydia().isInfected()){
    return true;
  }
  
  return false;
}


double EventChlamydiaDiagnosis::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionChlamydiaDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventChlamydiaDiagnosis::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionChlamydiaDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventChlamydiaDiagnosis::getTMax(const Person *pPerson)
{
  assert(pPerson != 0);
  double tb = pPerson->getDateOfBirth();
  
  assert(s_tMax > 0);
  return tb + s_tMax;
}

int EventChlamydiaDiagnosis::getD(Person *pPerson)
{
  // Person *pPerson = getPerson(0);
  
  int D1 = 0;
  int numRelations = pPerson->getNumberOfRelationships();
  double tDummy = 0;
  
  pPerson->startRelationshipIteration();
  for(int i = 0 ; i < numRelations; i++)
  {
    Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);
    if (pPartner->chlamydia().isDiagnosed())
      D1++;
  }
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
  
  return D1;
}


double EventChlamydiaDiagnosis::s_baseline = 0;
double EventChlamydiaDiagnosis::s_diagPartnersFactor = 0;
double EventChlamydiaDiagnosis::s_healthSeekingPropensityFactor = 0;
double EventChlamydiaDiagnosis::s_beta = 0;
double EventChlamydiaDiagnosis::s_tMax = 0;

void EventChlamydiaDiagnosis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("chlamydia.diagnosis.baseline", s_baseline)) ||
      !(r = config.getKeyValue("chlamydia.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.getKeyValue("chlamydia.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.getKeyValue("chlamydia.diagnosis.beta", s_beta)) ||
      !(r = config.getKeyValue("chlamydia.diagnosis.t_max", s_tMax))
  )
    abortWithMessage(r.getErrorString());
}

void EventChlamydiaDiagnosis::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("chlamydia.diagnosis.baseline", s_baseline)) ||
      !(r = config.addKey("chlamydia.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.addKey("chlamydia.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.addKey("chlamydia.diagnosis.beta", s_beta)) ||
      !(r = config.addKey("chlamydia.diagnosis.t_max", s_tMax)) 
  )
    abortWithMessage(r.getErrorString());
}

HazardFunctionChlamydiaDiagnosis::HazardFunctionChlamydiaDiagnosis(Person *pPerson, double baseline, double diagPartnersFactor, double healthSeekingPropensityFactor, double beta)
  : m_baseline(baseline), m_diagPartnersFactor(diagPartnersFactor), m_healthSeekingPropensityFactor(healthSeekingPropensityFactor), m_beta(beta)
{
  assert(pPerson != 0);
  m_pPerson = pPerson;
  
  double tinf = pPerson->chlamydia().getInfectionTime();
  double H = pPerson->getHealthSeekingPropensity();
  int D = EventChlamydiaDiagnosis::getD(pPerson);
  
  double A = baseline + diagPartnersFactor*D + healthSeekingPropensityFactor*H - beta*tinf;
  double B = beta;
  
  setAB(A, B);
}

// This implementation is not necessary for running, it is provided for testing purposes
double HazardFunctionChlamydiaDiagnosis::evaluate(double t)
{
  double tinf = m_pPerson->chlamydia().getInfectionTime();
  double H = m_pPerson->getHealthSeekingPropensity();
  int D = EventChlamydiaDiagnosis::getD(m_pPerson);
  
  return std::exp(m_baseline + m_diagPartnersFactor*D + m_healthSeekingPropensityFactor*H + m_beta*(t-tinf));
}

ConfigFunctions chlamydiaDiagnosisConfigFunctions(EventChlamydiaDiagnosis::processConfig, EventChlamydiaDiagnosis::obtainConfig, "EventChlamydiaDiagnosis");

JSONConfig chlamydiaDiagnosisJSONConfig(R"JSON(
    "EventChlamydiaDiagnosis": {
  "depends": null,
  "params": [
  ["chlamydia.diagnosis.baseline", 0],
  ["chlamydia.diagnosis.diagpartnersfactor", 0 ],
  ["chlamydia.diagnosis.beta", 0],
  ["chlamydia.diagnosis.healthseekingpropensityfactor", 0],
  ["chlamydia.diagnosis.t_max", 200]
  ],
            "info": [
  "TO DO"
            ]
})JSON");
