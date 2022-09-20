#include "eventchlamydiatransmission.h"

#include "jsonconfig.h"
#include "configfunctions.h"
#include "eventchlamydiaprogression.h"

using namespace std;

// When one of the involved persons dies before the event fires, the event is removed automatically
EventChlamydiaTransmission::EventChlamydiaTransmission(Person *pPerson1, Person *pPerson2): SimpactEvent(pPerson1, pPerson2)
{
	assert(pPerson1->chlamydia().isInfected() && !pPerson2->chlamydia().isInfected());
}

EventChlamydiaTransmission::~EventChlamydiaTransmission() {}

string EventChlamydiaTransmission::getDescription(double tNow) const
{
	return strprintf("Chlamydia Transmission event from %s to %s", getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventChlamydiaTransmission::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	writeEventLogStart(true, "Chlamydia transmission", tNow, pPerson1, pPerson2);
}

void EventChlamydiaTransmission::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	// Transmission from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// Person 1 should be infected , person 2 should not be infected yet
	assert(pPerson1->chlamydia().isInfected());
	assert(!pPerson2->chlamydia().isInfected());

	infectPerson(population, pPerson1, pPerson2, t);
}

void EventChlamydiaTransmission::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

    if (!(r = config.getKeyValue("chlamydiatransmission.hazard.a", s_a)) ||
    		!(r = config.getKeyValue("chlamydiatransmission.hazard.b", HazardFunctionChlamydiaTransmission::s_b)) ||
        	!(r = config.getKeyValue("chlamydiatransmission.hazard.t_max", s_tMax))
        )
        abortWithMessage(r.getErrorString());
}

void EventChlamydiaTransmission::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("chlamydiatransmission.hazard.a", s_a))||
			!(r = config.addKey("chlamydiatransmission.hazard.b", HazardFunctionChlamydiaTransmission::s_b)) ||
			!(r = config.addKey("chlamydiatransmission.hazard.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventChlamydiaTransmission::infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t)
{
	assert(!pTarget->chlamydia().isInfected());

	if (pOrigin == 0) // Seeding
		pTarget->chlamydia().setInfected(t, 0, Person_Chlamydia::Seed);
	else
	{
		assert(pOrigin->chlamydia().isInfected());
		pTarget->chlamydia().setInfected(t, pOrigin, Person_Chlamydia::Partner);
	}

	// Check relationships pTarget is in, and if partner not yet infected with chlamydia, schedule transmission event.
	int numRelations = pTarget->getNumberOfRelationships();
	pTarget->startRelationshipIteration();

	for (int i = 0; i < numRelations; i++)
	{
		double formationTime = -1;
		Person *pPartner = pTarget->getNextRelationshipPartner(formationTime);

		if (!pPartner->chlamydia().isInfected())
		{
			EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pTarget, pPartner);
			population.onNewEvent(pEvtTrans);
		}
	}

	// Schedule recovery event for newly infected person
	EventChlamydiaRecovery *pEvtRecovery = new EventChlamydiaRecovery(pTarget);
	population.onNewEvent(pEvtRecovery);
}


double EventChlamydiaTransmission::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionChlamydiaTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventChlamydiaTransmission::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pOrigin = getPerson(0);
	Person *pTarget = getPerson(1);
	double tMax = getTMax(pOrigin, pTarget);

	HazardFunctionChlamydiaTransmission h0(pOrigin, pTarget);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

// In case of dissolution of relationship, infection of susceptible person through other relationship,
// or cure of infected person, transmission event is no longer useful and needs to be discarded.
bool EventChlamydiaTransmission::isUseless(const PopulationStateInterface &population)
{
	// Transmission happens from pPerson1 to pPerson2
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	// If person2 was already infected, there is no sense in further transmission
	if (pPerson2->chlamydia().isInfected()) {
		return true;
	}
	// If person1 has been cured / has recovered, they can no longer transmit it
	if (!pPerson1->chlamydia().isInfected()) {
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

double EventChlamydiaTransmission::getTMax(const Person *pPerson1, const Person *pPerson2)
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

double EventChlamydiaTransmission::s_a = 0;
double EventChlamydiaTransmission::s_tMax = 200;


EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::HazardFunctionChlamydiaTransmission(const Person *pPerson1,
                                                                                      const Person *pPerson2)
    : HazardFunctionExp(getA(pPerson1, pPerson2), s_b)
{
}

EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::~HazardFunctionChlamydiaTransmission()
{
}

double EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::getA(const Person *pOrigin, const Person *pTarget)
{
    assert(pOrigin);
    assert(pTarget);
    return s_a - s_b * pOrigin->chlamydia().getInfectionTime();
}

double EventChlamydiaTransmission::HazardFunctionChlamydiaTransmission::s_b = 0;

ConfigFunctions chlamydiaTransmissionConfigFunctions(EventChlamydiaTransmission::processConfig,
														EventChlamydiaTransmission::obtainConfig,
														"EventChlamydiaTransmission");

JSONConfig chlamydiaTransmissionJSONConfig(R"JSON(
        "EventChlamydiaTransmission": {
            "depends": null,
            "params": [
				[ "chlamydiatransmission.hazard.a", 0 ],
				[ "chlamydiatransmission.hazard.b", 0 ],
				[ "chlamediatransmission.hazard.t_max", 200 ]
			],
            "info": [
				"These configuration parameters allow you to set the 'a' and 'b' in the hazard",
				" h = exp(a + b*(t-t_infected)"
            ]
        })JSON");
