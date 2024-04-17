#include "eventhsv2import.h"
#include "eventhsv2diagnosis.h"
#include "eventhsv2transmission.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "util.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

EventHSV2Import::EventHSV2Import()
{
}

EventHSV2Import::~EventHSV2Import()
{
}

double EventHSV2Import::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  double dt = m_hsv2SeedInterval;
  
  assert(dt >= 0);
  
  return dt;
}

string EventHSV2Import::getDescription(double tNow) const
{
  return "HSV2 importation";
}

void EventHSV2Import::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  writeEventLogStart(true, "HSV2 importation", tNow, 0, 0);
}

void EventHSV2Import::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRngGen = population.getRandomNumberGenerator();
  Person **ppPeople = population.getAllPeople();
  int numPeople = population.getNumberOfPeople();
  
  // Build pool of people not yet NG infected
  vector<Person *> possibleImports;
  
  for (int i = 0 ; i < numPeople ; i++)
  {
    Person *pPerson = ppPeople[i];
    bool infected = pPerson->hsv2().isInfected();
    
    if (!infected)
    {
      possibleImports.push_back(pPerson);
    }
  }
  
  // Get the actual number of infections that should be imported
  int numImports = m_hsv2ImportAmount;
  
  vector<Person *> imported;
  
  // Mark the specified number of people as imported case
  int countImported = 0;
  for (int i = 0 ; i < numImports && possibleImports.size() > 0 ; i++)
  {
    int poolSize = (int)possibleImports.size();
    int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());
    
    assert(seedIdx >= 0 && seedIdx < poolSize);
    
    Person *pPerson = possibleImports[seedIdx];
    
    EventHSV2Transmission::infectPerson(population, 0, pPerson, t);
    countImported++;
    
    // remove person from possible imports
    Person *pLastPerson = possibleImports[poolSize-1];
    possibleImports[seedIdx] = pLastPerson;
    possibleImports.resize(poolSize-1);
    
    imported.push_back(pPerson);
  }

  // Schedule next importation event
  EventHSV2Import *pEvtImport = new EventHSV2Import();
  population.onNewEvent(pEvtImport);
  
}

int EventHSV2Import::m_hsv2ImportAmount = 0;
double EventHSV2Import::m_hsv2SeedInterval = 0;

void EventHSV2Import::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  if(!(r = config.getKeyValue("hsv2import.amount", m_hsv2ImportAmount)) ||
     !(r = config.getKeyValue("hsv2import.interval", m_hsv2SeedInterval)))
     abortWithMessage(r.getErrorString());
}

void EventHSV2Import::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if(!(r = config.addKey("hsv2import.amount", m_hsv2ImportAmount)) ||
     !(r = config.addKey("hsv2import.interval", m_hsv2SeedInterval)))
     abortWithMessage(r.getErrorString());
}

ConfigFunctions hsv2importConfigFunctions(EventHSV2Import::processConfig, EventHSV2Import::obtainConfig, "EventHSV2Import");

JSONConfig hsv2importJSONConfig(R"JSON(
    "EventHSV2Import": {
  "depends": null,
  "params": [
  ["hsv2import.amount", 0],
  ["hsv2import.interval", 0]
  ],
            "info":[
  "TODO"
            ]
}
)JSON");
