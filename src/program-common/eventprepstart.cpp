#include "eventprepstart.h"

#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventprepscreening.h"
#include "jsonconfig.h"

using namespace std;

EventPrePStart::EventPrePStart(Person *pPerson) : SimpactEvent(pPerson) {}

EventPrePStart::~EventPrePStart() {}

string EventPrePStart::getDescription(double tNow) const
{
	return strprintf("PreP start event for %s", getPerson(0)->getName().c_str());
}

void EventPrePStart::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "prepstart", tNow, pPerson, 0);
}

void EventPrePStart::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(!pPerson->hiv().isInfected());

	pPerson->hiv().startPreP();

	// Schedule PreP screening immediately
	EventPrePScreening *pEvt = new EventPrePScreening(pPerson, true);
	population.onNewEvent(pEvt);

	// TODO schedule PreP dropout?
}

void EventPrePStart::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("prepstart.baseline", s_baseline)) ||
			!(r = config.getKeyValue("prepstart.beta", s_beta)) ||
			!(r = config.getKeyValue("prepstart.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventPrePStart::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("prepstart.baseline", s_baseline)) ||
			!(r = config.addKey("prepstart.beta", s_beta)) ||
			!(r = config.addKey("prepstart.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

double EventPrePStart::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionPrePStart h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventPrePStart::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionPrePStart h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventPrePStart::getTMax(const Person *pPerson)
{
	assert(pPerson != 0);
	double tb = pPerson->getDateOfBirth();

	assert(s_tMax > 0);
	return tb + s_tMax;
}

double EventPrePStart::s_baseline = 0;
double EventPrePStart::s_beta = 0;
double EventPrePStart::s_tMax = 200;

// exp(baseline + beta*(t-t_debut))
//
// = exp(A + B*t) with
//
//  A = baseline - beta*t_debut
//  B = beta
HazardFunctionPrePStart::HazardFunctionPrePStart(Person *pPerson, double baseline, double beta)
	: m_baseline(baseline), m_beta(beta)
{
	assert(pPerson != 0);
	m_pPerson = pPerson;

	double tdebut = pPerson->getDebutTime();
	double A = baseline - beta * tdebut;
	double B = beta;

	setAB(A, B);
}

ConfigFunctions prepStartConfigFunctions(EventPrePStart::processConfig, EventPrePStart::obtainConfig, "EventPrePStart");

JSONConfig prepStartJSONConfig(R"JSON(
        "EventPrePStart": {
            "depends": null,
            "params": [
                [ "prepstart.baseline", 0 ],
                [ "diagnosis.beta", 0 ],
                [ "diagnosis.t_max", 200 ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");

/*

#include "gslrandomnumbergenerator.h"
#include "util.h"
#include <iostream>

void EventDiagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	// Schedule an initial monitoring event right away! (the 'true' is for 'right away')
	EventMonitoring *pEvtMonitor = new EventMonitoring(pPerson, true);
	population.onNewEvent(pEvtMonitor);
}



void EventDiagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
{
	Person *pPerson = getPerson(0);

	// Infected partners (who possibly have a diagnosis event, of which
	// the hazard depends on the number of diagnosed partners), are also
	// affected!
	int numRel = pPerson->getNumberOfRelationships();

	pPerson->startRelationshipIteration();
	for (int i = 0 ; i < numRel ; i++)
	{
		double tDummy;
		Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);

		if (pPartner->hiv().isInfected())
			population.markAffectedPerson(pPartner);
	}
}
 *
 */
