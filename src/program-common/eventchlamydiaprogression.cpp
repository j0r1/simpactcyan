#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "eventchlamydiaprogression.h"

using namespace std;

EventChlamydiaRecovery::EventChlamydiaRecovery(Person *pPerson) : SimpactEvent(pPerson) {}

EventChlamydiaRecovery::~EventChlamydiaRecovery() {}

string EventChlamydiaRecovery::getDescription(double tNow) const
{
	return strprintf("Chlamydia recovery event for %s", getPerson(0)->getName().c_str());
}

void EventChlamydiaRecovery::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "chlamydia recovery", tNow, pPerson, 0);
}

void EventChlamydiaRecovery::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson->chlamydia().isInfected());

	// Mark the person as recovered from chlamydia
	pPerson->chlamydia().setRecovered(t);
}

void EventChlamydiaRecovery::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("chlamydiarecovery.baseline", s_baseline)) ||
			!(r = config.getKeyValue("chlamydiarecovery.beta", s_beta)) ||
			!(r = config.getKeyValue("chlamydiarecovery.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventChlamydiaRecovery::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("chlamydiarecovery.baseline", s_baseline)) ||
			!(r = config.addKey("chlamydiarecovery.beta", s_beta)) ||
			!(r = config.addKey("chlamydiarecovery.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

double EventChlamydiaRecovery::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionChlamydiaRecovery h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventChlamydiaRecovery::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionChlamydiaRecovery h0(pPerson, s_baseline, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventChlamydiaRecovery::getTMax(const Person *pPerson)
{
	assert(pPerson != 0);
	double tb = pPerson->getDateOfBirth();

	assert(s_tMax > 0);
	return tb + s_tMax;
}

double EventChlamydiaRecovery::s_baseline = 0;
double EventChlamydiaRecovery::s_beta = 0;
double EventChlamydiaRecovery::s_tMax = 200;

// exp(baseline + beta*(t-t_infected))
//
// = exp(A + B*t) with
//
//  A = baseline - beta*t_infected
//  B = beta
HazardFunctionChlamydiaRecovery::HazardFunctionChlamydiaRecovery(Person *pPerson, double baseline, double beta):
		m_baseline(baseline), m_beta(beta)
{
	assert(pPerson != 0);
	m_pPerson = pPerson;

	double tinf = pPerson->chlamydia().getInfectionTime();

	double A = baseline - beta*tinf;
	double B = beta;

	setAB(A, B);
}

ConfigFunctions chlamydiaRecoveryConfigFunctions(EventChlamydiaRecovery::processConfig, EventChlamydiaRecovery::obtainConfig, "EventChlamydiaRecovery");

JSONConfig chlamydiaRecoveryJSONConfig(R"JSON(
        "EventChlamydiaRecovery": {
            "depends": null,
            "params": [
                [ "chlamydiarecovery.baseline", 0 ],
                [ "chlamydiarecovery.beta", 0 ],
                [ "chlamydiarecovery.t_max", 200 ]
            ],
            "info": [
                "When a person gets infected with gonorrhea, a recovery event is ",
                "scheduled of which the fire time is determined by the following hazard:",
                "",
                " h = exp(baseline + beta*t)."
            ]
        })JSON");
