#include "eventhivseed.h"
#include "eventaidsmortality.h"
#include "eventhivtransmission.h"
#include "eventchronicstage.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>
#include <cmath>

using namespace std;

HIVSeedEventSettings::HIVSeedEventSettings()
{
	m_fraction_chronic = -1;
	m_fraction_aids = -1;
	m_fraction_final_aids = -1;
	m_fraction_diagnosed = -1;
}

EventHIVSeed::EventHIVSeed()
{
}

EventHIVSeed::~EventHIVSeed()
{
}

double EventHIVSeed::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	return EventSeedBase::getNewInternalTimeDifference(s_settings, pRndGen, pState);
}

string EventHIVSeed::getDescription(double tNow) const
{
	return "HIV seeding";
}

void EventHIVSeed::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	writeEventLogStart(true, "HIV seeding", tNow, 0, 0);
}

void EventHIVSeed::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	EventSeedBase::fire(s_settings, t, pState, EventHIVTransmission::infectPerson);
}

HIVSeedEventSettings EventHIVSeed::s_settings;

void EventHIVSeed::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	EventSeedBase::processConfig(s_settings, config, pRndGen, "hivseed");

	bool_t r;

	if (!(r = config.getKeyValue("hivseed.fractionchronic", s_settings.m_fraction_chronic)) ||
		!(r = config.getKeyValue("hivseed.fractionaids", s_settings.m_fraction_aids)) ||
		!(r = config.getKeyValue("hivseed.fractionfinalaids", s_settings.m_fraction_final_aids)) ||
		!(r = config.getKeyValue("hivseed.fractiondiagnosed", s_settings.m_fraction_diagnosed))
	    )
		abortWithMessage(r.getErrorString());

}

void EventHIVSeed::obtainConfig(ConfigWriter &config)
{
	EventSeedBase::obtainConfig(s_settings, config, "hivseed");

	bool_t r;

	if (!(r = config.addKey("hivseed.fractionchronic", s_settings.m_fraction_chronic)) ||
		!(r = config.addKey("hivseed.fractionaids", s_settings.m_fraction_aids)) ||
		!(r = config.addKey("hivseed.fractionfinalaids", s_settings.m_fraction_final_aids)) ||
		!(r = config.addKey("hivseed.fractiondiagnosed", s_settings.m_fraction_diagnosed))
		)
		abortWithMessage(r.getErrorString());
}

ConfigFunctions hivseedingConfigFunctions(EventHIVSeed::processConfig, EventHIVSeed::obtainConfig, "EventHIVSeed");

// The 0 is the default seed time; HIV seeding by default is at the start of the simulation
JSONConfig hivseedingJSONConfig(R"JSON(
        "EventHIVSeed": { 
            "depends": null,
            "params": [
				["hivseed.time", 0],
				["hivseed.type", "fraction", [ "fraction", "amount"] ],
				["hivseed.age.min", 0], 
				["hivseed.age.max", 1000],
				["hivseed.gender", "any", [ "any", "male", "female"] ],
				["hivseed.fractionchronic", 0],
				["hivseed.fractionaids", 0],
				["hivseed.fractionfinalaids", 0],
				["hivseed.fractiondiagnosed", 0]
			],
            "info": [ 
                "Controls when the initial HIV seeders are introduced, and who those seeders",
                "are. First, the possible seeders are chosen from the population, based on the",
                "specified minimum and maximum ages, and on the specified gender.",
                "",
                "The specified time says when the seeding event should take place. Note that",
                "if the time is negative, no seeders will be introduced since the event will ",
                "be ignored (simulation time starts at t = 0)."
            ]
        }, 
        "EventHIVSeed_fraction": {
            "depends": [ "hivseed", "hivseed.type", "fraction" ],
            "params": [
                [ "hivseed.fraction", 0.2 ]
            ],
            "info": [
                "From the people who possibly can be seeded with HIV, the specified fraction",
                "will be marked as infected."
            ]
        },

        "EventHIVSeed_number": {
            "depends": [ "hivseed", "hivseed.type", "amount" ],
            "params": [
                [ "hivseed.amount", 1 ],
                [ "hivseed.stop.short", "yes", [ "yes", "no" ] ]
            ],
            "info": [
                "From the people who possibly can be seeded with HIV, the specified amount",
                "will be marked as infected. If the 'hivseed.stop.short' parameter is set to",
                "'yes' and there were not enough people that could be infected, the program",
                "will abort."
            ]
        })JSON");
