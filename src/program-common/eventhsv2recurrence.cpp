#include "eventhsv2recurrence.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventHSV2Recurrence::EventHSV2Recurrence(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventHSV2Recurrence::~EventHSV2Recurrence()
{
}

string EventHSV2Recurrence::getDescription(double tNow) const
{
  return strprintf("HSV2 recurrence event for %s", getPerson(0)->getName().c_str());
}

void EventHSV2Recurrence::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "HSV2 recurrence", tNow, pPerson, 0);
}

void EventHSV2Recurrence::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  
  Person_HSV2::HSV2DiseaseStage diseaseStage = pPerson->hsv2().getDiseaseStage();
  if(diseaseStage == Person_HSV2::Asymptomatic){
    pPerson->hsv2().setInRecurrentStage(t);
  }else if(diseaseStage == Person_HSV2::Recurrent){
    pPerson->hsv2().setInAsymptomaticStage(t);
    
  }
  
  // Schedule new recurrence/asympt event
  EventHSV2Recurrence *pEvtRecurr = new EventHSV2Recurrence(pPerson, false);
  population.onNewEvent(pEvtRecurr);
}

double EventHSV2Recurrence::m_sheddingFreq = 0;
double EventHSV2Recurrence::m_nCycles = 0;
double EventHSV2Recurrence::m_chronicHIV = 0;
double EventHSV2Recurrence::m_AIDS = 0;
double EventHSV2Recurrence::m_redTreat = 0;

bool EventHSV2Recurrence::isWillingToTreatSTI(GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);
  
  // Coin toss
  double x = pRndGen->pickRandomDouble();
  if (x < pPerson->getTreatAcceptanceThreshold())
    return true;
  
  return false;  
}

double EventHSV2Recurrence::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &popoulation = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  
  assert(pPerson != 0);
  assert(getNumberOfPersons() == 1);
  assert(pPerson->hsv2().isInfected());
  
  assert(m_sheddingFreq > 0);
  assert(m_nCycles > 0);
  
  double dt = 0;
  
  Person_HSV2::HSV2DiseaseStage diseaseStage = pPerson->hsv2().getDiseaseStage();
  Person_HIV::InfectionStage hivStage = pPerson->hiv().getInfectionStage();
  Person_HSV2::HSV2SymptomStatus symptomStatus = pPerson->hsv2().getSymptoms();

  // If willing to get treated, symptomatic, and diagnosed -> set reduction factor
  if( (isWillingToTreatSTI(pRndGen) && symptomStatus==Person_HSV2::Symptoms && pPerson->hsv2().isDiagnosed()) ){
    m_redTreat = m_redTreat;
  } else {
    m_redTreat = 1;
  }
  // else{
  //   writeEventLogStart(true, "(hsv2 treatment)", t, pPerson, 0);
  // }
  
  if(diseaseStage == Person_HSV2::Asymptomatic){
    
    if(hivStage == Person_HIV::NoInfection){
      // dt = 365*((1-m_sheddingFreq)/m_nCycles) / 365;
      dt = ((1-m_sheddingFreq*m_redTreat)/m_nCycles);
    }else if(hivStage == Person_HIV::Acute || hivStage == Person_HIV::Chronic){
      dt = ((1-(m_chronicHIV*m_sheddingFreq*m_redTreat))/m_nCycles);
    }else if(hivStage == Person_HIV::AIDS || hivStage == Person_HIV::AIDSFinal){
      dt = ((1-(m_AIDS*m_sheddingFreq*m_redTreat))/m_nCycles);
    }
  } else if(diseaseStage == Person_HSV2::Recurrent){
    
    if(hivStage == Person_HIV::NoInfection){
      dt = (m_sheddingFreq/m_nCycles) * m_redTreat;
    }else if(hivStage == Person_HIV::Acute || hivStage == Person_HIV::Chronic){
      dt = ((m_chronicHIV*m_sheddingFreq)/m_nCycles) * m_redTreat;
    }else if(hivStage == Person_HIV::AIDS || hivStage == Person_HIV::AIDSFinal){
      dt = ((m_AIDS*m_sheddingFreq)/m_nCycles) * m_redTreat;
    }

  }
  
  return dt;
  
}

void EventHSV2Recurrence::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  
  if(!(r = config.getKeyValue("hsv2shedding.freq", m_sheddingFreq, 0)) ||
     !(r = config.getKeyValue("hsv2shedding.cycles", m_nCycles, 0)) ||
     !(r = config.getKeyValue("hsv2shedding.hivfactor", m_chronicHIV, 0)) ||
     !(r = config.getKeyValue("hsv2shedding.aidsfactor", m_AIDS, 0)) ||
     !(r = config.getKeyValue("hsv2shedding.treatfactor", m_redTreat, 0))
  )
    abortWithMessage(r.getErrorString());
}

void EventHSV2Recurrence::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if(!(r = config.addKey("hsv2shedding.freq", m_sheddingFreq)) ||
     !(r = config.addKey("hsv2shedding.cycles", m_nCycles)) ||
     !(r = config.addKey("hsv2shedding.hivfactor", m_chronicHIV)) ||
     !(r = config.addKey("hsv2shedding.aidsfactor", m_AIDS)) ||
     !(r = config.addKey("hsv2shedding.treatfactor", m_redTreat))
  )
    abortWithMessage(r.getErrorString());
}

ConfigFunctions hsv2RecurrenceConfigFunctions(EventHSV2Recurrence::processConfig, EventHSV2Recurrence::obtainConfig, "EventHSV2Recurrence");

JSONConfig hsv2RecurrenceJSONConfig(R"JSON(
    "EventHSV2Recurrence": { 
  "depends": null,
  "params": [ ["hsv2shedding.freq", 0.14 ],
              ["hsv2shedding.cycles", 4],
              ["hsv2shedding.hivfactor", 1],
              ["hsv2shedding.aidsfactor", 1],
              ["hsv2shedding.treatfactor", 1]
        ],
            "info": [ "TODO" ]
})JSON");
  
  

