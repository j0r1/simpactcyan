#include "eventhivimport.h"
#include "eventhivtransmission.h"
#include "eventchronicstage.h"
#include "eventdiagnosis.h"
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

EventHIVImport::EventHIVImport()
{
}

EventHIVImport::~EventHIVImport()
{
}

double EventHIVImport::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  const SimpactPopulation &population = SIMPACTPOPULATION(pState);
  double dt = m_seedInterval;
  
  assert(dt >= 0);
  
  return dt;
}

string EventHIVImport::getDescription(double tNow) const
{
  return "HIV importation";
}

void EventHIVImport::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  writeEventLogStart(true, "HIV importation", tNow, 0, 0);
}

void EventHIVImport::fire(Algorithm *pAlgorithm, State *pState, double t)
{
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  GslRandomNumberGenerator *pRngGen = population.getRandomNumberGenerator();
  Person **ppPeople = population.getAllPeople();
  int numPeople = population.getNumberOfPeople();
  
  // Build pool of people not yet HIV infected
  vector<Person *> possibleImports;
  
  for (int i = 0 ; i < numPeople ; i++)
  {
    Person *pPerson = ppPeople[i];
    bool infected = pPerson->hiv().isInfected();
    
    if (!infected)
    {
      possibleImports.push_back(pPerson);
    }
  }
  
  // Get the actual number of infections that should be imported
  int numImports = m_importAmount;
  
  vector<Person *> imported;
  
  // Mark the specified number of people as imported case
  int countImported = 0;
  for (int i = 0 ; i < numImports && possibleImports.size() > 0 ; i++)
  {
    int poolSize = (int)possibleImports.size();
    int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());
    
    assert(seedIdx >= 0 && seedIdx < poolSize);
    
    Person *pPerson = possibleImports[seedIdx];
    
    EventHIVTransmission::infectPerson(population, 0, pPerson, t, false);
    countImported++;
    
    // remove person from possible imports
    Person *pLastPerson = possibleImports[poolSize-1];
    possibleImports[seedIdx] = pLastPerson;
    possibleImports.resize(poolSize-1);
    
    imported.push_back(pPerson);
  }
  
  // Schedule chronic stage and diagnosis event
  for (int i = 0 ; i < imported.size() ; i++)
  {
    Person *pPerson = imported[i];
    
    EventChronicStage *pEvtChronic = new EventChronicStage(pPerson);
    population.onNewEvent(pEvtChronic);
    
    EventDiagnosis *pEvtDiag = new EventDiagnosis(pPerson);
    population.onNewEvent(pEvtDiag);
  }
  
  // Schedule next importation event
  EventHIVImport *pEvtImport = new EventHIVImport();
  population.onNewEvent(pEvtImport);
  
}

int EventHIVImport::m_importAmount = 0;
double EventHIVImport::m_seedInterval = 0;

void EventHIVImport::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  bool_t r;
  if(!(r = config.getKeyValue("hivimport.amount", m_importAmount)) ||
     !(r = config.getKeyValue("hivimport.interval", m_seedInterval)))
    abortWithMessage(r.getErrorString());
}

void EventHIVImport::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if(!(r = config.addKey("hivimport.amount", m_importAmount)) ||
     !(r = config.addKey("hivimport.interval", m_seedInterval)))
    abortWithMessage(r.getErrorString());
}

ConfigFunctions hivimportConfigFunctions(EventHIVImport::processConfig, EventHIVImport::obtainConfig, "EventHIVImport");

JSONConfig hivimportJSONConfig(R"JSON(
  "EventHIVImport": {
  "depends": null,
  "params": [
    ["hivimport.amount", 0],
    ["hivimport.interval", 0]
  ],
            "info":[
              "TODO"
            ]
}
)JSON");
