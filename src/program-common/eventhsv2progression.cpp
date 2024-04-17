#include "eventhsv2progression.h"
#include "eventhsv2transmission.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "person_hsv2.h"
#include "eventhsv2recurrence.h"

using namespace std;

EventHSV2Progression::EventHSV2Progression(Person *pPerson) : SimpactEvent(pPerson) {}

EventHSV2Progression::~EventHSV2Progression() {}

string EventHSV2Progression::getDescription(double tNow) const
{
  return strprintf("HSV2 progression (asymptomatic stage) event for %s", getPerson(0)->getName().c_str());
}

void EventHSV2Progression::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "HSV2 progression", tNow, pPerson, 0);
}

void EventHSV2Progression::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  
  pPerson->hsv2().progress(t, false);
  // pPerson->writeToHSV2TreatLog();
  
  // Schedule recurrence event
  EventHSV2Recurrence *pEvtRecurr = new EventHSV2Recurrence(pPerson, false);
  population.onNewEvent(pEvtRecurr);
  
}

bool EventHSV2Progression::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if person not infected or already past the primary stage
  Person *pPerson1 = getPerson(0);
  Person_HSV2::HSV2DiseaseStage diseaseStage = pPerson1->hsv2().getDiseaseStage();
  if(!pPerson1->hsv2().isInfected() || diseaseStage == Person_HSV2::Asymptomatic || diseaseStage == Person_HSV2::Recurrent){
    return true;
  }
  return false;
}

void EventHSV2Progression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  assert(pRndGen != 0);
  
  delete s_hPrimaryStageDurationDistribution;
  s_hPrimaryStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "hsv2progression.primarystageduration");
  
}

void EventHSV2Progression::obtainConfig(ConfigWriter &config)
{
  assert(s_hPrimaryStageDurationDistribution);
  addDistributionToConfig(s_hPrimaryStageDurationDistribution, config, "hsv2progression.primarystageduration");
}

double EventHSV2Progression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  assert(pPerson != 0);
  assert(getNumberOfPersons() == 1);
  assert(pPerson->hsv2().isInfected());
  
  double dt = 0;
  
  Person_HSV2::HSV2DiseaseStage diseaseStage = pPerson->hsv2().getDiseaseStage();
  if(diseaseStage == Person_HSV2::Primary) {
    dt = s_hPrimaryStageDurationDistribution->pickNumber();
  }
  
  return dt;
}

ProbabilityDistribution *EventHSV2Progression::s_hPrimaryStageDurationDistribution = 0;

ConfigFunctions hsv2ProgressionConfigFunctions(EventHSV2Progression::processConfig, EventHSV2Progression::obtainConfig, "EventHSV2Progression");

JSONConfig hsv2ProgressionJSONConfig(R"JSON(
  "EventHSV2Progression": {
  "depends": null,
  "params": [
    ["hsv2progression.primarystageduration.dist", "distTypes", ["fixed", [ [ "value", 0.05 ] ] ] ]
  ],
            "info": [
              "TODO"
            ]
})JSON");
