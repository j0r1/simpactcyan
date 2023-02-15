#include "eventgonorrheadiagnosis.h"
#include "eventgonorrheaprogression.h"
#include "eventgonorrheatransmission.h"
#include "person_gonorrhea.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventGonorrheaDiagnosis::EventGonorrheaDiagnosis(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventGonorrheaDiagnosis::~EventGonorrheaDiagnosis()
{
}

string EventGonorrheaDiagnosis::getDescription(double tNow) const
{
  return strprintf("Gonorrhea diagnosis event for %s", getPerson(0)->getName().c_str());
}

void EventGonorrheaDiagnosis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "gonorrhea diagnosis", tNow, pPerson, 0);
}

void EventGonorrheaDiagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
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
    
    if (pPartner->gonorrhea().isInfected())
      population.markAffectedPerson(pPartner);
  }
  
#ifndef NDEBUG
  // Double check that the iteration is done
  double tDummy;
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

bool EventGonorrheaDiagnosis::isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);

  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTreatAcceptanceThreshold())
    return true;
  
  return false;  
}

void EventGonorrheaDiagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
  Person *pPerson = getPerson(0);
  
  assert(pPerson->gonorrhea().isInfected());
  
  // Mark person as diagnosed
  pPerson->gonorrhea().diagnose(t);
  double tLast = pPerson->getTimeLastSTI();
  pPerson->increaseSTIDiagnoseCount(t, tLast);

  // If accepting treatment or on PrEP --> immediate recovery
  if(isWillingToTreatSTI(t, pRndGen) || pPerson->hiv().isOnPreP())
  {
    pPerson->gonorrhea().progress(t, true);
    pPerson->writeToGonorrheaTreatLog();
    writeEventLogStart(true, "(gonorrhea treatment)", t, pPerson, 0);
    
    // Schedule new transmission event from infectious partners if person becomes susceptible again --> MOVE TO PERSON_CHLAMYDIA??
    Person_Gonorrhea::GonorrheaDiseaseStage diseaseStage = pPerson->gonorrhea().getDiseaseStage();
    if (diseaseStage == Person_Gonorrhea::Susceptible) {
      int numRelations = pPerson->getNumberOfRelationships();
      pPerson->startRelationshipIteration();
      
      for (int i = 0; i < numRelations; i++)
      {
        double formationTime = -1;
        Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
        if (pPartner->gonorrhea().isInfectious())
        {
          EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pPartner, pPerson);
          population.onNewEvent(pEvtTrans);
        }
      }
    }
    
  }
  
}

bool EventGonorrheaDiagnosis::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if natural clearance occurs before scheduled diagnosis
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->gonorrhea().isInfected()){
    return true;
  }
  
  return false;
}


double EventGonorrheaDiagnosis::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionGonorrheaDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.calculateInternalTimeInterval(t0, dt);
}

double EventGonorrheaDiagnosis::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
  // for the diagnosis event that should be scheduled right after an STI screening event
  if(m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  Person *pPerson = getPerson(0);
  double tMax = getTMax(pPerson);
  
  HazardFunctionGonorrheaDiagnosis h0(pPerson, s_baseline, s_diagPartnersFactor, s_healthSeekingPropensityFactor, s_beta);
  TimeLimitedHazardFunction h(h0, tMax);
  
  return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventGonorrheaDiagnosis::getTMax(const Person *pPerson)
{
  assert(pPerson != 0);
  double tb = pPerson->getDateOfBirth();
  
  assert(s_tMax > 0);
  return tb + s_tMax;
}

int EventGonorrheaDiagnosis::getD(Person *pPerson)
{
  // Person *pPerson = getPerson(0);
  
  int D1 = 0;
  int numRelations = pPerson->getNumberOfRelationships();
  double tDummy = 0;
  
  pPerson->startRelationshipIteration();
  for(int i = 0 ; i < numRelations; i++)
  {
    Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);
    if (pPartner->gonorrhea().isDiagnosed())
      D1++;
  }
  assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
  
  return D1;
}

double EventGonorrheaDiagnosis::s_baseline = 0;
double EventGonorrheaDiagnosis::s_diagPartnersFactor = 0;
double EventGonorrheaDiagnosis::s_healthSeekingPropensityFactor = 0;
double EventGonorrheaDiagnosis::s_beta = 0;
double EventGonorrheaDiagnosis::s_tMax = 0;

void EventGonorrheaDiagnosis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if (!(r = config.getKeyValue("gonorrhea.diagnosis.baseline", s_baseline)) ||
      !(r = config.getKeyValue("gonorrhea.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.getKeyValue("gonorrhea.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.getKeyValue("gonorrhea.diagnosis.beta", s_beta)) ||
      !(r = config.getKeyValue("gonorrhea.diagnosis.t_max", s_tMax))
  )
    abortWithMessage(r.getErrorString());
}

void EventGonorrheaDiagnosis::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("gonorrhea.diagnosis.baseline", s_baseline)) ||
      !(r = config.addKey("gonorrhea.diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
      !(r = config.addKey("gonorrhea.diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
      !(r = config.addKey("gonorrhea.diagnosis.beta", s_beta)) ||
      !(r = config.addKey("gonorrhea.diagnosis.t_max", s_tMax)) 
  )
    abortWithMessage(r.getErrorString());
}

HazardFunctionGonorrheaDiagnosis::HazardFunctionGonorrheaDiagnosis(Person *pPerson, double baseline, double diagPartnersFactor, double healthSeekingPropensityFactor, double beta)
  : m_baseline(baseline), m_diagPartnersFactor(diagPartnersFactor), m_healthSeekingPropensityFactor(healthSeekingPropensityFactor), m_beta(beta)
{
  assert(pPerson != 0);
  m_pPerson = pPerson;
  
  double tinf = pPerson->gonorrhea().getInfectionTime();
  double H = pPerson->getHealthSeekingPropensity();
  int D = EventGonorrheaDiagnosis::getD(pPerson);
  
  double A = baseline + diagPartnersFactor*D + healthSeekingPropensityFactor*H - beta*tinf;
  double B = beta;
  
  setAB(A, B);
}

// This implementation is not necessary for running, it is provided for testing purposes
double HazardFunctionGonorrheaDiagnosis::evaluate(double t)
{
  double tinf = m_pPerson->gonorrhea().getInfectionTime();
  double H = m_pPerson->getHealthSeekingPropensity();
  int D = EventGonorrheaDiagnosis::getD(m_pPerson);
  
  return std::exp(m_baseline + m_diagPartnersFactor*D + m_healthSeekingPropensityFactor*H + m_beta*(t-tinf));
}

ConfigFunctions gonorrheaDiagnosisConfigFunctions(EventGonorrheaDiagnosis::processConfig, EventGonorrheaDiagnosis::obtainConfig, "EventGonorrheaDiagnosis");

JSONConfig gonorrheaDiagnosisJSONConfig(R"JSON(
  "EventGonorrheaDiagnosis": {
    "depends": null,
    "params": [
      ["gonorrhea.diagnosis.baseline", 0],
      ["gonorrhea.diagnosis.diagpartnersfactor", 0 ],
      ["gonorrhea.diagnosis.beta", 0],
      ["gonorrhea.diagnosis.healthseekingpropensityfactor", 0],
      ["gonorrhea.diagnosis.t_max", 200]
    ],
              "info": [
                "TO DO"
              ]
})JSON");
