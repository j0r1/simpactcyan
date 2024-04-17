#include "eventchlamydiaimport.h"
#include "eventchlamydiadiagnosis.h"
#include "eventchlamydiatransmission.h"
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

EventChlamydiaImport::EventChlamydiaImport()
{
}

EventChlamydiaImport::~EventChlamydiaImport()
{
}

double EventChlamydiaImport::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  double dt = m_chlamydiaSeedInterval;
  
  assert(dt >= 0);
  
  return dt;
}

string EventChlamydiaImport::getDescription(double tNow) const
{
  return "Chlamydia importation";
}

void EventChlamydiaImport::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  writeEventLogStart(true, "Chlamydia importation", tNow, 0, 0);
}

void EventChlamydiaImport::fire(Algorithm *pAlgorithm, State *pState, double t)
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
    bool infected = pPerson->chlamydia().isInfected();
    
    if (!infected)
    {
      possibleImports.push_back(pPerson);
    }
  }
  
  // Get the actual number of infections that should be imported
  int numImports = m_chlamydiaImportAmount;
  
  vector<Person *> imported;
  
  // Mark the specified number of people as imported case
  int countImported = 0;
  for (int i = 0 ; i < numImports && possibleImports.size() > 0 ; i++)
  {
    int poolSize = (int)possibleImports.size();
    int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());
    
    assert(seedIdx >= 0 && seedIdx < poolSize);
    
    Person *pPerson = possibleImports[seedIdx];
    
    EventChlamydiaTransmission::infectPerson(population, 0, pPerson, t);
    countImported++;
    
    // remove person from possible imports
    Person *pLastPerson = possibleImports[poolSize-1];
    possibleImports[seedIdx] = pLastPerson;
    possibleImports.resize(poolSize-1);
    
    imported.push_back(pPerson);
  }
  
  // Schedule next importation event
  EventChlamydiaImport *pEvtImport = new EventChlamydiaImport();
  population.onNewEvent(pEvtImport);
  
}

int EventChlamydiaImport::m_chlamydiaImportAmount = 0;
double EventChlamydiaImport::m_chlamydiaSeedInterval = 0;

void EventChlamydiaImport::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  if(!(r = config.getKeyValue("chlamydiaimport.amount", m_chlamydiaImportAmount)) ||
     !(r = config.getKeyValue("chlamydiaimport.interval", m_chlamydiaSeedInterval)))
     abortWithMessage(r.getErrorString());
}

void EventChlamydiaImport::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if(!(r = config.addKey("chlamydiaimport.amount", m_chlamydiaImportAmount)) ||
     !(r = config.addKey("chlamydiaimport.interval", m_chlamydiaSeedInterval)))
     abortWithMessage(r.getErrorString());
}

ConfigFunctions chlamydiaimportConfigFunctions(EventChlamydiaImport::processConfig, EventChlamydiaImport::obtainConfig, "EventChlamydiaImport");

JSONConfig chlamydiaimportJSONConfig(R"JSON(
    "EventChlamydiaImport": {
  "depends": null,
  "params": [
  ["chlamydiaimport.amount", 0],
  ["chlamydiaimport.interval", 0]
  ],
            "info":[
  "TODO"
            ]
}
)JSON");
