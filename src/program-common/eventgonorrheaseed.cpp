#include "eventgonorrheaseed.h"
#include "eventgonorrheaimport.h"
#include "configfunctions.h"
#include "eventgonorrheatransmission.h"

#include "jsonconfig.h"

using namespace std;

EventGonorrheaSeed::EventGonorrheaSeed() {}

EventGonorrheaSeed::~EventGonorrheaSeed() {}

double EventGonorrheaSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	return EventSeedBase::getNewInternalTimeDifference(s_settings, pRndGen, pState);
}

string EventGonorrheaSeed::getDescription(double tNow) const
{
	return "Gonorrhea seeding";
}

void EventGonorrheaSeed::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	writeEventLogStart(true, "Gonorrhea seeding", tNow, 0, 0);
}

void EventGonorrheaSeed::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	EventSeedBase::fire(s_settings, t, pState, EventGonorrheaTransmission::infectPerson);
  
  SimpactPopulation &population = SIMPACTPOPULATION(pState);
  
  // Schedule next importation event
  EventGonorrheaImport *pEvtImport = new EventGonorrheaImport();
  population.onNewEvent(pEvtImport);
}

SeedEventSettings EventGonorrheaSeed::s_settings;

void EventGonorrheaSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventSeedBase::processConfig(s_settings, config, pRndGen, "gonorrheaseed");
}

void EventGonorrheaSeed::obtainConfig(ConfigWriter &config)
{
	EventSeedBase::obtainConfig(s_settings, config, "gonorrheaseed");
}

ConfigFunctions gonorrheaSeedingConfigFunctions(EventGonorrheaSeed::processConfig, EventGonorrheaSeed::obtainConfig, "EventGonorrheaSeed");

// The -1 is the default seed time; a negative value means it's disabled
JSONConfig gonorrheaseedingJSONConfig(EventSeedBase::getJSONConfigText("EventGonorrheaSeeding", "gonorrheaseed", "Gonorrhea", -1));

