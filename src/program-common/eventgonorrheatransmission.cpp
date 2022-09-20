#include "eventgonorrheatransmission.h"

#include "jsonconfig.h"
#include "configfunctions.h"
#include "eventgonorrhearecovery.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventGonorrheaTransmission::EventGonorrheaTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
	assert(pPerson1->gonorrhea().isInfected() && !pPerson2->gonorrhea().isInfected());
}

EventGonorrheaTransmission::~EventGonorrheaTransmission() {}

string EventGonorrheaTransmission::getDescription(double tNow) const
{
	return strprintf("Gonorrhea Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventGonorrheaTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "Gonorrhea transmission", tNow, pPerson1, pPerson2);
}

void EventGonorrheaTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	// Transmission from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// Person 1 should be infected , person 2 should not be infected yet
	assert(pPerson1->gonorrhea().isInfected());
	assert(!pPerson2->gonorrhea().isInfected());

	infectPerson(population, pPerson1, pPerson2, t);
}

void EventGonorrheaTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

    if (!(r = config.getKeyValue("gonorrheatransmission.hazard.a", s_a)) ||
    		!(r = config.getKeyValue("gonorrheatransmission.hazard.b", HazardFunctionGonorrheaTransmission::s_b)) ||
        	!(r = config.getKeyValue("gonorrheatransmission.hazard.t_max", s_tMax))
        )
        abortWithMessage(r.getErrorString());
}

void EventGonorrheaTransmission::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("gonorrheatransmission.hazard.a", s_a))||
			!(r = config.addKey("gonorrheatransmission.hazard.b", HazardFunctionGonorrheaTransmission::s_b)) ||
			!(r = config.addKey("gonorrheatransmission.hazard.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}


void EventGonorrheaTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
	assert(!pTarget->gonorrhea().isInfected());

	if (pOrigin == 0) // Seeding
		pTarget->gonorrhea().setInfected(t, 0, Person_Gonorrhea::Seed);
	else
	{
		assert(pOrigin->gonorrhea().isInfected());
		pTarget->gonorrhea().setInfected(t, pOrigin, Person_Gonorrhea::Partner);
	}

	// Check relationships pTarget is in, and if partner not yet infected with gonorrhea, schedule transmission event.
	int numRelations = pTarget->getNumberOfRelationships();
	pTarget->startRelationshipIteration();

	for (int i = 0; i < numRelations; i++)
	{
		double formationTime = -1;
		Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);

		if (!pPartner->gonorrhea().isInfected())
		{
			EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pTarget, pPartner);
			population.onNewEvent(pEvtTrans);
		}
	}

	// Schedule recovery event for newly infected person
	EventGonorrheaRecovery *pEvtRecovery = new EventGonorrheaRecovery(pTarget);
	population.onNewEvent(pEvtRecovery);
}

double EventGonorrheaTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionGonorrheaTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventGonorrheaTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionGonorrheaTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventGonorrheaTransmission::isUseless(const PopulationStateInterface &population)
{
	// Transmission happens from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// If person2 was already infected with gonorrhea, there is no sense in further transmission
	if (pPerson2->gonorrhea().isInfected()) {
		return true;
	}
	// If person1 has been cured of gonorrhea, they can no longer transmit it
	if (!pPerson1->gonorrhea().isInfected()) {
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

//double calculateHazardFactor(const SimpactPopulation &population, double t0);

double EventGonorrheaTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
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

double EventGonorrheaTransmission::s_a = 0;
double EventGonorrheaTransmission::s_tMax = 200;

EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::HazardFunctionGonorrheaTransmission(const Person *pPerson1,
                                                                                      const Person *pPerson2)
    : HazardFunctionExp(getA(pPerson1, pPerson2), s_b)
{
}

EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::~HazardFunctionGonorrheaTransmission()
{
}

double EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::getA(const Person *pOrigin, const Person *pTarget)
{
    assert(pOrigin);
    assert(pTarget);
    return s_a - s_b * pOrigin->gonorrhea().getInfectionTime();
}

double EventGonorrheaTransmission::HazardFunctionGonorrheaTransmission::s_b = 0;

ConfigFunctions gonorrheaTransmissionConfigFunctions(EventGonorrheaTransmission::processConfig,
														EventGonorrheaTransmission::obtainConfig,
														"EventGonorrheaTransmission");

JSONConfig gonorrheaTransmissionJSONConfig(R"JSON(
        "EventGonorrheaTransmission": {
            "depends": null,
            "params": [
				[ "gonorrheatransmission.hazard.a", 0 ],
				[ "gonorrheatransmission.hazard.b", 0 ],
				[ "gonorrheatransmission.hazard.t_max", 200 ]
			],
            "info": [
				"These configuration parameters allow you to set the 'a' and 'b' in the hazard",
				" h = exp(a + b*(t-t_infected)"
            ]
        })JSON");
