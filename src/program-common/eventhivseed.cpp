#include "eventhivseed.h"
#include "eventaidsmortality.h"
#include "eventhivtransmission.h"
#include "eventchronicstage.h"
#include "eventdiagnosis.h"
#include "eventaidsstage.h"
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
	// Check that this event is only carried out once
	if (s_settings.m_seeded)
		abortWithMessage("Can only seed population once!");

	s_settings.m_seeded = true;

	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	GslRandomNumberGenerator *pRngGen = population.getRandomNumberGenerator();
	Person **ppPeople = population.getAllPeople();
	int numPeople = population.getNumberOfPeople();

	// Build pool of people which can be seeded
	assert(s_settings.m_seedMinAge >= 0 && s_settings.m_seedMaxAge >= s_settings.m_seedMinAge);
	assert(s_settings.m_seedGender == SeedEventSettings::Any ||
		   s_settings.m_seedGender == SeedEventSettings::Male ||
		   s_settings.m_seedGender == SeedEventSettings::Female);

	vector<Person *> possibleSeeders;

	for (int i = 0 ; i < numPeople ; i++)
	{
		Person *pPerson = ppPeople[i];
		double age = pPerson->getAgeAt(t);

		if (age >= s_settings.m_seedMinAge && age <= s_settings.m_seedMaxAge)
		{
			if ( s_settings.m_seedGender == SeedEventSettings::Any ||
			     (s_settings.m_seedGender == SeedEventSettings::Male && pPerson->isMan()) ||
				 (s_settings.m_seedGender == SeedEventSettings::Female && pPerson->isWoman()) )
				possibleSeeders.push_back(pPerson);
		}
	}

	// Get the actual number of people that should be seeded
	int numSeeders = 0;

	if (!s_settings.m_useFraction) // we've already specified the actual number
		numSeeders = s_settings.m_seedAmount;
	else
	{
		// We've specified a fraction, use a binomial distribution to obtain the number of people
		// to be seeded
		assert(s_settings.m_seedFraction >= 0 && s_settings.m_seedFraction <= 1.0);

		numSeeders = pRngGen->pickBinomialNumber(s_settings.m_seedFraction, possibleSeeders.size());
	}

	int numChronic = pRngGen->pickBinomialNumber(s_settings.m_fraction_chronic, numSeeders);
	int numAids = pRngGen->pickBinomialNumber(s_settings.m_fraction_aids, numSeeders);
	int numFinalAids = pRngGen->pickBinomialNumber(s_settings.m_fraction_final_aids, numSeeders);
	int numDiagnosed = pRngGen->pickBinomialNumber(s_settings.m_fraction_diagnosed, numSeeders);

	vector<Person *> seeded;

	// Mark the specified number of people as seeders
	int countSeeded = 0;
	for (int i = 0 ; i < numSeeders && possibleSeeders.size() > 0 ; i++)
	{
		int poolSize = (int)possibleSeeders.size();
		int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());

		assert(seedIdx >= 0 && seedIdx < poolSize);

		Person *pPerson = possibleSeeders[seedIdx];

		EventHIVTransmission::infectPerson(population, 0, pPerson, t, false); // 0 means seed
		countSeeded++;

		// remove the person from the possibleSeeders
		// To do so, we're going to move the last person to the seeder position and
		// shrink the possibleSeeders array
		Person *pLastPerson = possibleSeeders[poolSize-1];
		possibleSeeders[seedIdx] = pLastPerson;
		possibleSeeders.resize(poolSize-1);

		seeded.push_back(pPerson);
	}

	assert(numChronic + numAids + numFinalAids <= countSeeded);

	while (numChronic > 0 || numAids > 0 || numFinalAids > 0) {
		// Pick random seeder
		int poolSize = (int)seeded.size();
		int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());

		assert(seedIdx >= 0 && seedIdx < poolSize);

		Person *pPerson = seeded[seedIdx];
		// Check if person still in acute stage
		if (pPerson->hiv().getInfectionStage() == Person_HIV::InfectionStage::Acute) {

			if (numChronic > 0) {
				EventChronicStage *pEvt = new EventChronicStage(pPerson);
				pEvt->fire(pAlgorithm, pState, t);
				numChronic--;
			} else if (numAids > 0) {
				EventAIDSStage *pEvt = new EventAIDSStage(pPerson, false);
				pEvt->fire(pAlgorithm, pState, t);
				numAids --;
			} else if (numFinalAids > 0) {
				// TODO
				numFinalAids--;
			}
		}

	}

	// Schedule chronic stage event for all those still in acute stage
	for (int i = 0; i < seeded.size(); i++) {
		Person *pPerson = seeded[i];
		if (pPerson->hiv().getInfectionStage() == Person_HIV::InfectionStage::Acute) {
			EventChronicStage *pEvt = new EventChronicStage(pPerson);
			population.onNewEvent(pEvt);
		}
	}

	assert(numDiagnosed <= countSeeded);
	while (numDiagnosed > 0) {
		// Pick random seeder
		int poolSize = (int)seeded.size();
		int seedIdx = (int)((double)poolSize * pRngGen->pickRandomDouble());

		assert(seedIdx >= 0 && seedIdx < poolSize);
		Person *pPerson = seeded[seedIdx];

		// Check that person has not yet been diagnosed
		if (!pPerson->hiv().isDiagnosed()) {
			EventDiagnosis *pEvt = new EventDiagnosis(pPerson);
			pEvt->fire(pAlgorithm, pState, t);
			numDiagnosed--;
		}
	}

	// Schedule diagnosis event for all those undiagnosed
	for (int i = 0; i < seeded.size(); i++) {
		Person *pPerson = seeded[i];
		if (!pPerson->hiv().isDiagnosed()) {
			EventDiagnosis *pEvt = new EventDiagnosis(pPerson);
			population.onNewEvent(pEvt);
		}
	}

	if (!s_settings.m_useFraction && s_settings.m_stopOnShort && countSeeded != s_settings.m_seedAmount)
		abortWithMessage(strprintf("Could not HSV2 seed the requested amount of people: %d were seeded, but %d requested", countSeeded, s_settings.m_seedAmount));

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

//JSONConfig hivseedingJSONConfig(EventSeedBase::getJSONConfigText("EventHIVSeeding", "hivseed", "HIV", 0));
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
            "depends": [ "EventHIVSeed", "hivseed.type", "fraction" ],
            "params": [
                [ "hivseed.fraction", 0.2 ]
            ],
            "info": [
                "From the people who possibly can be seeded with HIV, the specified fraction",
                "will be marked as infected."
            ]
        },

        "EVENTNAMETEMPLATE_number": {
            "depends": [ "EventHIVSeed", "hivseed.type", "amount" ],
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
        }
)JSON");

