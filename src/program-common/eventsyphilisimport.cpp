#include "eventsyphilisimport.h"
#include "eventsyphilisdiagnosis.h"
#include "eventsyphilistransmission.h"
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

EventSyphilisImport::EventSyphilisImport()
{
}

EventSyphilisImport::~EventSyphilisImport()
{
}

double EventSyphilisImport::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  double dt = m_syphilisSeedInterval;
  
  assert(dt >= 0);
  
  return dt;
}

string EventSyphilisImport::getDescription(double tNow) const
{
  return "Syphilis importation";
}

void EventSyphilisImport::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  writeEventLogStart(true, "Syphilis importation", tNow, 0, 0);
}

void EventSyphilisImport::fire(Algorithm *pAlgorithm, State *pState, double t)
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
    bool infected = pPerson->syphilis().isInfected();
    
    if (!infected)
    {
      possibleImports.push_back(pPerson);
    }
  }
  
  // Get the actual number of infections that should be imported
  int numImports = m_syphilisImportAmount;
  
  vector<Person *> imported;
  
  // Mark the specified number of people as imported case
  int countImported = 0;
  for (int i = 0 ; i < numImports && possibleImports.size() > 0 ; i++)
  {
    int poolSize = (int)possibleImports.size();
    int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());
    
    assert(seedIdx >= 0 && seedIdx < poolSize);
    
    Person *pPerson = possibleImports[seedIdx];
    
    EventSyphilisTransmission::infectPerson(population, 0, pPerson, t);
    countImported++;
    
    // remove person from possible imports
    Person *pLastPerson = possibleImports[poolSize-1];
    possibleImports[seedIdx] = pLastPerson;
    possibleImports.resize(poolSize-1);
    
    imported.push_back(pPerson);
  }
  
  // Schedule next importation event
  EventSyphilisImport *pEvtImport = new EventSyphilisImport();
  population.onNewEvent(pEvtImport);
  
}

int EventSyphilisImport::m_syphilisImportAmount = 0;
double EventSyphilisImport::m_syphilisSeedInterval = 0;

void EventSyphilisImport::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  if(!(r = config.getKeyValue("syphilisimport.amount", m_syphilisImportAmount)) ||
     !(r = config.getKeyValue("syphilisimport.interval", m_syphilisSeedInterval)))
     abortWithMessage(r.getErrorString());
}

void EventSyphilisImport::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if(!(r = config.addKey("syphilisimport.amount", m_syphilisImportAmount)) ||
     !(r = config.addKey("syphilisimport.interval", m_syphilisSeedInterval)))
     abortWithMessage(r.getErrorString());
}

ConfigFunctions syphilisimportConfigFunctions(EventSyphilisImport::processConfig, EventSyphilisImport::obtainConfig, "EventSyphilisImport");

JSONConfig syphilisimportJSONConfig(R"JSON(
    "EventSyphilisImport": {
  "depends": null,
  "params": [
  ["syphilisimport.amount", 0],
  ["syphilisimport.interval", 0]
  ],
            "info":[
  "TODO"
            ]
}
)JSON");
