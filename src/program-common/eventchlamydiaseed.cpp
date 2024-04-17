#include "eventchlamydiaseed.h"
#include "eventchlamydiaimport.h"
#include "configfunctions.h"
#include "eventchlamydiatransmission.h"

#include "jsonconfig.h"

using namespace std;

EventChlamydiaSeed::EventChlamydiaSeed() {}

EventChlamydiaSeed::~EventChlamydiaSeed() {}

string EventChlamydiaSeed::getDescription(double tNow) const
{
	return "Chlamydia seeding";
}

void EventChlamydiaSeed::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	writeEventLogStart(true, "Chlamydia seeding", tNow, 0, 0);
}

void EventChlamydiaSeed::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	EventSeedBase::fire(s_settings, t, pState, EventChlamydiaTransmission::infectPerson);
  
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  // Schedule next importation event
  EventChlamydiaImport *pEvtImport = new EventChlamydiaImport();
  population.onNewEvent(pEvtImport);
}

SeedEventSettings EventChlamydiaSeed::s_settings;

void EventChlamydiaSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventSeedBase::processConfig(s_settings, config, pRndGen, "chlamydiaseed");
}

void EventChlamydiaSeed::obtainConfig(ConfigWriter &config)
{
	EventSeedBase::obtainConfig(s_settings, config, "chlamydiaseed");
}

double EventChlamydiaSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	return EventSeedBase::getNewInternalTimeDifference(s_settings, pRndGen, pState);
}

ConfigFunctions chlamydiaSeedingConfigFunctions(EventChlamydiaSeed::processConfig, EventChlamydiaSeed::obtainConfig, "EventChlamydiaSeed");

// The -1 is the default seed time; a negative value means it's disabled
JSONConfig chlamydiaseedingJSONConfig(EventSeedBase::getJSONConfigText("EventChlamydiaSeeding", "chlamydiaseed", "Chlamydia", -1));
