#include "eventgonorrheaimport.h"
#include "eventchronicstage.h"
#include "eventgonorrheadiagnosis.h"
#include "eventgonorrheatransmission.h"
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

EventGonorrheaImport::EventGonorrheaImport()
{
}

EventGonorrheaImport::~EventGonorrheaImport()
{
}

double EventGonorrheaImport::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  double dt = m_gonorrheaSeedInterval;
  
  assert(dt >= 0);
  
  return dt;
}

string EventGonorrheaImport::getDescription(double tNow) const
{
  return "Gonorrhea importation";
}

void EventGonorrheaImport::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  writeEventLogStart(true, "Gonorrhea importation", tNow, 0, 0);
}

void EventGonorrheaImport::fire(Algorithm *pAlgorithm, State *pState, double t)
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
    bool infected = pPerson->gonorrhea().isInfected();
    
    if (!infected)
    {
      possibleImports.push_back(pPerson);
    }
  }
  
  // Get the actual number of infections that should be imported
  int numImports = m_gonorrheaImportAmount;
  
  vector<Person *> imported;
  
  // Mark the specified number of people as imported case
  int countImported = 0;
  for (int i = 0 ; i < numImports && possibleImports.size() > 0 ; i++)
  {
    int poolSize = (int)possibleImports.size();
    int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());
    
    assert(seedIdx >= 0 && seedIdx < poolSize);
    
    Person *pPerson = possibleImports[seedIdx];
    
    EventGonorrheaTransmission::infectPerson(population, 0, pPerson, t);
    countImported++;
    
    // remove person from possible imports
    Person *pLastPerson = possibleImports[poolSize-1];
    possibleImports[seedIdx] = pLastPerson;
    possibleImports.resize(poolSize-1);
    
    imported.push_back(pPerson);
  }
  
  // Schedule diagnosis event
  for (int i = 0 ; i < imported.size() ; i++)
  {
    Person *pPerson = imported[i];
    
    EventGonorrheaDiagnosis *pEvtDiag = new EventGonorrheaDiagnosis(pPerson);
    population.onNewEvent(pEvtDiag);

  }
  
  // Schedule next importation event
  EventGonorrheaImport *pEvtImport = new EventGonorrheaImport();
  population.onNewEvent(pEvtImport);
  
}

int EventGonorrheaImport::m_gonorrheaImportAmount = 0;
double EventGonorrheaImport::m_gonorrheaSeedInterval = 0;

void EventGonorrheaImport::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  if(!(r = config.getKeyValue("gonorrheaimport.amount", m_gonorrheaImportAmount)) ||
     !(r = config.getKeyValue("gonorrheaimport.interval", m_gonorrheaSeedInterval)))
     abortWithMessage(r.getErrorString());
}

void EventGonorrheaImport::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if(!(r = config.addKey("gonorrheaimport.amount", m_gonorrheaImportAmount)) ||
     !(r = config.addKey("gonorrheaimport.interval", m_gonorrheaSeedInterval)))
     abortWithMessage(r.getErrorString());
}

ConfigFunctions gonorrheaimportConfigFunctions(EventGonorrheaImport::processConfig, EventGonorrheaImport::obtainConfig, "EventGonorrheaImport");

JSONConfig gonorrheaimportJSONConfig(R"JSON(
    "EventGonorrheaImport": {
  "depends": null,
  "params": [
  ["gonorrheaimport.amount", 0],
  ["gonorrheaimport.interval", 0]
  ],
            "info":[
  "TODO"
            ]
}
)JSON");
