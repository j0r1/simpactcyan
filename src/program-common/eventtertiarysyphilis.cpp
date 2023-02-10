#include "eventtertiarysyphilis.h"
#include "eventsyphilistransmission.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configdistributionhelper.h"
#include "configsettings.h"
#include "configwriter.h"
#include "person_syphilis.h"
#include "util.h"
#include <iostream>

using namespace std;

EventTertiarySyphilis::EventTertiarySyphilis(Person *pPerson, bool scheduleImmediately) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately)
{
}

EventTertiarySyphilis::~EventTertiarySyphilis()
{
}

string EventTertiarySyphilis::getDescription(double tNow) const
{
  Person *pPerson = getPerson(0);
  return strprintf("Tertiary syphilis stage for %s", pPerson->getName().c_str());
}

void EventTertiarySyphilis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "tertiary syphilis", tNow, pPerson, 0);
}

void EventTertiarySyphilis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  assert(getNumberOfPersons() == 1);
  
  Person *pPerson = getPerson(0);
  assert(pPerson->syphilis().getDiseaseStage() == Person_Syphilis::Latent);
  
  pPerson->syphilis().setInTertiaryStage(t);  
  
  
}

bool EventTertiarySyphilis::isUseless(const PopulationStateInterface &population)
{
  Person *pPerson1 = getPerson(0);
  
  // If person already in tertiary stage
  if(pPerson1->syphilis().getDiseaseStage() == Person_Syphilis::Tertiary){
    return true;
  }
  
  // If person no longer infected
  if(!(pPerson1->syphilis().isInfected())){
    return true;
  }
  
  return false;
}

void EventTertiarySyphilis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  assert(pRndGen != 0);
  
  delete s_pTertiaryStageTimeDistribution;
  s_pTertiaryStageTimeDistribution = getDistributionFromConfig(config, pRndGen, "syphilisprogression.tertiarystage");
  
}

void EventTertiarySyphilis::obtainConfig(ConfigWriter &config)
{
  assert(s_pTertiaryStageTimeDistribution);
  addDistributionToConfig(s_pTertiaryStageTimeDistribution, config, "syphilisprogression.tertiarystage");
  
}


double EventTertiarySyphilis::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  
  assert(pPerson != 0);
  assert(getNumberOfPersons() == 1);
  assert(pPerson->syphilis().isInfected());
  
  // For progression from latent to tertiary
  if (m_scheduleImmediately)
  {
    double hour = 1.0/(365.0*24.0); // an hour in a unit of a year
    return hour;
  }
  
  double tertiaryTime = 0;
  tertiaryTime = s_pTertiaryStageTimeDistribution->pickNumber();
  assert(tertiaryTime > 0);
  
  double tEvt = pPerson->syphilis().getInfectionTime() + tertiaryTime;
  double dt = tEvt - population.getTime();
  
  assert(dt >= 0);
  
  return dt;
  
}

ProbabilityDistribution *EventTertiarySyphilis::s_pTertiaryStageTimeDistribution = 0;


ConfigFunctions syphilisTertiaryConfigFunction(EventTertiarySyphilis::processConfig, EventTertiarySyphilis::obtainConfig, "EventTertiarySyphilis");

JSONConfig syphilisTertiaryJSONConfig(R"JSON(
  "EventTertiarySyphilis": {
  "depends": null,
  "params": [
    ["syphilisprogression.tertiarystage.dist", "distTypes", ["fixed", [ [ "value", 20]]] ]
  ],
            "info":[
              "TODO"
            ]
})JSON");




