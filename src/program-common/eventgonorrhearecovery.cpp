#include "eventgonorrhearecovery.h"

#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "configfunctions.h"

using namespace std;

EventGonorrheaRecovery::EventGonorrheaRecovery(Person *pPerson) : SimpactEvent(pPerson) {}

EventGonorrheaRecovery::~EventGonorrheaRecovery() {}

string EventGonorrheaRecovery::getDescription(double tNow) const
{
	return strprintf("Gonorrhea recovery event for %s", getPerson(0)->getName().c_str());
}

void EventGonorrheaRecovery::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "gonorrhea recovery", tNow, pPerson, 0);
}

void EventGonorrheaRecovery::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson->gonorrhea().isInfected());

	// Mark the person as recovered from gonorrhea
	pPerson->gonorrhea().setRecovered(t);
}

void EventGonorrheaRecovery::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("gonorrhearecovery.baseline", s_baseline)) ||
			!(r = config.getKeyValue("gonorrhearecovery.beta", s_beta)) ||
			!(r = config.getKeyValue("gonorrhearecovery.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventGonorrheaRecovery::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("gonorrhearecovery.baseline", s_baseline)) ||
			!(r = config.addKey("gonorrhearecovery.beta", s_beta)) ||
			!(r = config.addKey("gonorrhearecovery.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

double EventGonorrheaRecovery::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionGonorrheaRecovery h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventGonorrheaRecovery::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionGonorrheaRecovery h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventGonorrheaRecovery::getTMax(const Person *pPerson)
{
	assert(pPerson != 0);
	double tb = pPerson->getDateOfBirth();

	assert(s_tMax > 0);
	return tb + s_tMax;
}

double EventGonorrheaRecovery::s_baseline = 0;
double EventGonorrheaRecovery::s_beta = 0;
double EventGonorrheaRecovery::s_tMax = 200;

// exp(baseline + beta*(t-t_infected))
//
// = exp(A + B*t) with
//
//  A = baseline - beta*t_infected
//  B = beta
HazardFunctionGonorrheaRecovery::HazardFunctionGonorrheaRecovery(Person *pPerson, double baseline, double beta):
		m_baseline(baseline), m_beta(beta)
{
	assert(pPerson != 0);
	m_pPerson = pPerson;

	double tinf = pPerson->gonorrhea().getInfectionTime();

	double A = baseline - beta*tinf;
	double B = beta;

	setAB(A, B);
}

ConfigFunctions gonorrheaRecoveryConfigFunctions(EventGonorrheaRecovery::processConfig, EventGonorrheaRecovery::obtainConfig, "EventGonorrheaRecovery");

JSONConfig gonorrheaRecoveryJSONConfig(R"JSON(
        "EventGonorrheaRecovery": {
            "depends": null,
            "params": [
                [ "gonorrhearecovery.baseline", 0 ],
                [ "gonorrhearecovery.beta", 0 ],
                [ "gonorrhearecovery.t_max", 200 ]
            ],
            "info": [
                "When a person gets infected with gonorrhea, a recovery event is ",
                "scheduled of which the fire time is determined by the following hazard:",
                "",
                " h = exp(baseline + beta*t)."
            ]
        })JSON");
