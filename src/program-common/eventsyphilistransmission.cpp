#include "eventsyphilistransmission.h"

#include "configfunctions.h"
#include "eventsyphilisprogression.h"
#include "jsonconfig.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventSyphilisTransmission::EventSyphilisTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
	assert(pPerson1->syphilis().isInfectious() && !pPerson2->syphilis().isInfected());
}

EventSyphilisTransmission::~EventSyphilisTransmission() {}

string EventSyphilisTransmission::getDescription(double tNow) const
{
	return strprintf("Syphilis Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventSyphilisTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "Syphilis transmission", tNow, pPerson1, pPerson2);
}

void EventSyphilisTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	// Transmission from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// Person 1 should be infectious , person 2 should not be infected yet
	assert(pPerson1->chlamydia().isInfectious());
	assert(!pPerson2->chlamydia().isInfected());

	infectPerson(population, pPerson1, pPerson2, t);
}

void EventSyphilisTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

    if (!(r = config.getKeyValue("syphilistransmission.hazard.a", s_a)) ||
    		!(r = config.getKeyValue("syphilistransmission.hazard.b", HazardFunctionSyphilisTransmission::s_b)) ||
        	!(r = config.getKeyValue("syphilistransmission.hazard.t_max", s_tMax))
        )
        abortWithMessage(r.getErrorString());
}

void EventSyphilisTransmission::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("syphilistransmission.hazard.a", s_a))||
			!(r = config.addKey("syphilistransmission.hazard.b", HazardFunctionSyphilisTransmission::s_b)) ||
			!(r = config.addKey("syphilistransmission.hazard.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventSyphilisTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
	assert(!pTarget->syphilis().isInfected());

	if (pOrigin == 0) // Seeding
		pTarget->syphilis().setInfected(t, 0, Person_Syphilis::Seed);
	else
	{
		assert(pOrigin->syphilis().isInfectious());
		pTarget->syphilis().setInfected(t, pOrigin, Person_Syphilis::Partner);
	}

	// Schedule progression event for newly infected person
	// Exposed --> Primary
	EventSyphilisProgression *pEvtProgression = new EventSyphilisProgression(pTarget);
	population.onNewEvent(pEvtProgression);
}

double EventSyphilisTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionSyphilisTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventSyphilisTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionSyphilisTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or recovery / cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventSyphilisTransmission::isUseless(const PopulationStateInterface &population)
{
	// Transmission happens from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// If person2 is already infected, there is no sense in further transmission
	if (pPerson2->syphilis().isInfected()) {
		return true;
	}
	// If person1 is no longer infectious, they can no longer transmit it
	if (!pPerson1->syphilis().isInfectious()) {
		return true;
	}

	// If relationship is over, transmission event is useless
	if (!pPerson1->hasRelationshipWith(pPerson2))
	{
		assert(!pPerson2->hasRelationshipWith(pPerson1));
		return true;
	}

	// Make sure the two lists are consistent: if person1 has a relationship with person2, person2
	// should also have a relationship with person1
	assert(pPerson2->hasRelationshipWith(pPerson1));

	return false;
}

double EventSyphilisTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
{
    assert(pPerson1 != 0 && pPerson2 != 0);

    double tb1 = pPerson1->getDateOfBirth();
    double tb2 = pPerson2->getDateOfBirth();

    double tMax = tb1;

    if (tb2 < tMax)
        tMax = tb2;

    assert(s_tMax > 0);
    tMax += s_tMax;
    return tMax;
}

double EventSyphilisTransmission::s_a = 0;
double EventSyphilisTransmission::s_tMax = 200;

EventSyphilisTransmission::HazardFunctionSyphilisTransmission::HazardFunctionSyphilisTransmission(const Person *pPerson1,
                                                                                      const Person *pPerson2)
    : HazardFunctionExp(getA(pPerson1, pPerson2), s_b)
{}

EventSyphilisTransmission::HazardFunctionSyphilisTransmission::~HazardFunctionSyphilisTransmission() {}

double EventSyphilisTransmission::HazardFunctionSyphilisTransmission::getA(const Person *pOrigin, const Person *pTarget)
{
    assert(pOrigin);
    assert(pTarget);
    return s_a - s_b * pOrigin->syphilis().getInfectionTime();
}

double EventSyphilisTransmission::HazardFunctionSyphilisTransmission::s_b = 0;

ConfigFunctions syphilisTransmissionConfigFunctions(EventSyphilisTransmission::processConfig,
														EventSyphilisTransmission::obtainConfig,
														"EventSyphilisTransmission");

JSONConfig syphilisTransmissionJSONConfig(R"JSON(
        "EventSyphilisTransmission": {
            "depends": null,
            "params": [
				[ "syphilistransmission.hazard.a", 0 ],
				[ "syphilistransmission.hazard.b", 0 ],
				[ "syphilistransmission.hazard.t_max", 200 ]
			],
            "info": [
				"These configuration parameters allow you to set the 'a' and 'b' in the hazard",
				" h = exp(a + b*(t-t_infected)"
            ]
        })JSON");
