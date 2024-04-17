#include "eventroutinetesting.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventdiagnosis.h"
#include "eventchlamydiadiagnosis.h"
#include "eventgonorrheadiagnosis.h"
#include "eventsyphilisdiagnosis.h"
#include "eventhsv2diagnosis.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "probabilitydistribution.h"

using namespace std;

EventRoutineTesting::EventRoutineTesting(Person *pPerson, bool scheduleImmediately): SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately) {}

EventRoutineTesting::~EventRoutineTesting() {}

string EventRoutineTesting::getDescription(double tNow) const
{
  return strprintf("Routine testing event for %s", getPerson(0)->getName().c_str());
}

void EventRoutineTesting::writeLogs(const SimpactPopulation &pop, double tNow) const
{
  Person *pPerson = getPerson(0);
  writeEventLogStart(true, "routinetesting", tNow, pPerson, 0);
}

void EventRoutineTesting::fire(Algorithm *pAlgorithm, State *pState, double t){
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  Person *pPerson = getPerson(0);
  
  // Enabled for each infection separately?
  // If infected + not diagnosed -> diagnosed immediately
  if (pPerson->hiv().isInfected() && (!pPerson->hiv().isDiagnosed())) {
    EventDiagnosis *pEvtDiagnosis = new EventDiagnosis(pPerson, true);
    population.onNewEvent(pEvtDiagnosis); // stopPrEP() is done in diagnosis event
  }
  
  if(pPerson->chlamydia().isInfected()){
    EventChlamydiaDiagnosis *pEvtChlamydiaDiagnosis = new EventChlamydiaDiagnosis(pPerson, true);
    population.onNewEvent(pEvtChlamydiaDiagnosis);
  }
  
  if(pPerson->gonorrhea().isInfected()){
    EventGonorrheaDiagnosis *pEvtGonorrheaDiagnosis = new EventGonorrheaDiagnosis(pPerson, true);
    population.onNewEvent(pEvtGonorrheaDiagnosis);
  }
  
  if(pPerson->syphilis().isInfected()){
    EventSyphilisDiagnosis *pEvtSyphilisDiagnosis = new EventSyphilisDiagnosis(pPerson, true);
    population.onNewEvent(pEvtSyphilisDiagnosis);
  }
  
  if(pPerson->hsv2().isInfected()){
    EventHSV2Diagnosis *pEvtHSV2Diagnosis = new EventHSV2Diagnosis(pPerson, true);
    population.onNewEvent(pEvtHSV2Diagnosis);
  }
  
  // Schedule new screening
  EventRoutineTesting *pNewTesting = new EventRoutineTesting(pPerson);
  population.onNewEvent(pNewTesting);
  
}

bool EventRoutineTesting::isUseless(const PopulationStateInterface &pop)
{
  // if diagnosed with all infections
  Person *pPerson = getPerson(0);
  
  if (pPerson->hiv().isDiagnosed() && pPerson->chlamydia().isDiagnosed() && pPerson->gonorrhea().isDiagnosed() && pPerson->syphilis().isDiagnosed()
        && pPerson->hsv2().isDiagnosed()) {
    return true;
  }
  
  // also useless if person no longer sexually active
  if (!(pPerson->isSexuallyActive())){
    return true;
  }
  
  return false;
}

void EventRoutineTesting::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  delete s_pRoutineTestingIntervalDistribution;
  s_pRoutineTestingIntervalDistribution = getDistributionFromConfig(config, pRndGen, "routinetesting.interval");
}

void EventRoutineTesting::obtainConfig(ConfigWriter &config)
{
  assert(s_pRoutineTestingIntervalDistribution);
  addDistributionToConfig(s_pRoutineTestingIntervalDistribution, config, "routinetesting.interval");
}

double EventRoutineTesting::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
  // This is for the screening event that should be scheduled right after the
  // prep start event event
  if (m_scheduleImmediately)
  {
    double minute = 1.0/(365.0*24.0*60.0); // a minute in a unit of a year
    return minute;
  }
  
  double dt = s_pRoutineTestingIntervalDistribution->pickNumber();
  
  assert(dt >= 0);
  
  return dt;
}

ProbabilityDistribution *EventRoutineTesting::s_pRoutineTestingIntervalDistribution = 0;

ConfigFunctions routineTestingConfigFunctions(EventRoutineTesting::processConfig, EventRoutineTesting::obtainConfig, "EventRoutineTesting");

JSONConfig routineTestingJSONConfig(R"JSON(
    "EventRoutineTesting": {
  "depends": null,
  "params": [ ["routinetesting.interval.dist", "distTypes", ["fixed", [ [ "value", 0.25 ] ] ] ] ],
        "info": [
  "TODO"
        ]
})JSON");


