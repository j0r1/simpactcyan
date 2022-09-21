#include "eventsyphilisseed.h"

#include "configfunctions.h"
#include "eventsyphilistransmission.h"
#include "jsonconfig.h"

using namespace std;

EventSyphilisSeed::EventSyphilisSeed() {}

EventSyphilisSeed::~EventSyphilisSeed() {}

string EventSyphilisSeed::getDescription(double tNow) const
{
	return "Syphilis seeding";
}

void EventSyphilisSeed::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	writeEventLogStart(true, "Syphilis seeding", tNow, 0, 0);
}

void EventSyphilisSeed::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	EventSeedBase::fire(s_settings, t, pState, EventSyphilisTransmission::infectPerson);
}

SeedEventSettings EventSyphilisSeed::s_settings;


void EventSyphilisSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventSeedBase::processConfig(s_settings, config, pRndGen, "syphilisseed");
}

void EventSyphilisSeed::obtainConfig(ConfigWriter &config)
{
	EventSeedBase::obtainConfig(s_settings, config, "syphilisseed");
}

double EventSyphilisSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	return EventSeedBase::getNewInternalTimeDifference(s_settings, pRndGen, pState);
}

ConfigFunctions syphilisSeedingConfigFunctions(EventSyphilisSeed::processConfig, EventSyphilisSeed::obtainConfig, "EventSyphilisSeed");

// The -1 is the default seed time; a negative value means it's disabled
JSONConfig syphilisseedingJSONConfig(EventSeedBase::getJSONConfigText("EventSyphilisSeeding", "syphilisseed", "Syphilis", -1));

