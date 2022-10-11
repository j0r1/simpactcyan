#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventprepoffered.h"
#include "eventprepscreening.h"
#include "jsonconfig.h"

using namespace std;

EventPrePOffered::EventPrePOffered(Person *pPerson) : SimpactEvent(pPerson) {}

EventPrePOffered::~EventPrePOffered() {}

string EventPrePOffered::getDescription(double tNow) const
{
	return strprintf("PreP offered to %s", getPerson(0)->getName().c_str());
}

void EventPrePOffered::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "prepoffered", tNow, pPerson, 0);
}

void EventPrePOffered::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	Person *pPerson = getPerson(0);

	assert(!pPerson->hiv().isInfected());

	// Check if person is willing to start PreP
	bool isWillingToTakePreP = false;
	double rn = pRndGen->pickRandomDouble();
	if (rn < pPerson->hiv().getPrePAcceptanceThreshold()) {
		isWillingToTakePreP = true;
	}

	if (isWillingToTakePreP) {
		// Start PreP
		pPerson->hiv().startPreP();
		// Schedule PreP screening immediately
		EventPrePScreening *pEvt = new EventPrePScreening(pPerson, true);

		// TODO schedule PreP dropout?
		population.onNewEvent(pEvt);
	} else {
		// Schedule new offering event
		EventPrePOffered *pEvt = new EventPrePOffered(pPerson);
		population.onNewEvent(pEvt);
	}
}

void EventPrePOffered::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("prepoffered.baseline", s_baseline)) ||
			!(r = config.getKeyValue("prepoffered.diagpartnersfactor", s_diagpartnersfactor)) ||
			!(r = config.getKeyValue("prepoffered.beta", s_beta)) ||
			!(r = config.getKeyValue("prepoffered.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

void EventPrePOffered::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("prepoffered.baseline", s_baseline)) ||
			!(r = config.addKey("prepoffered.diagpartnersfactor", s_diagpartnersfactor)) ||
			!(r = config.addKey("prepoffered.beta", s_beta)) ||
			!(r = config.addKey("prepoffered.t_max", s_tMax))
		)
		abortWithMessage(r.getErrorString());
}

bool EventPrePOffered::isUseless(const PopulationStateInterface &pop)
{
	// PreP offering event becomes useless if person has been diagnosed with HIV
	Person *pPerson = getPerson(0);

	if (pPerson->hiv().isDiagnosed()) {
		return true;
	}

	return false;
}

double EventPrePOffered::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionPrePOffered h0(pPerson, s_baseline, s_diagpartnersfactor, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.calculateInternalTimeInterval(t0, dt);
}

double EventPrePOffered::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionPrePOffered h0(pPerson, s_baseline, s_diagpartnersfactor, s_beta);
	TimeLimitedHazardFunction h(h0, tMax);

	return h.solveForRealTimeInterval(t0, Tdiff);
}

double EventPrePOffered::getTMax(const Person *pPerson)
{
	assert(pPerson != 0);
	double tb = pPerson->getDateOfBirth();

	assert(s_tMax > 0);
	return tb + s_tMax;
}

double EventPrePOffered::s_baseline = 0;
double EventPrePOffered::s_diagpartnersfactor = 0;
double EventPrePOffered::s_beta = 0;
double EventPrePOffered::s_tMax = 200;

// exp(baseline + beta*(t-t_debut))
//
// = exp(A + B*t) with
//
//  A = baseline + s_diatpartnersfactor*Dp - beta*t_debut
//  B = beta
//
// wit Dp the number of diagnosed partners
HazardFunctionPrePOffered::HazardFunctionPrePOffered(Person *pPerson, double baseline, double diagpartnersfactor, double beta)
	: m_baseline(baseline), m_diagpartnersfactor(diagpartnersfactor), m_beta(beta)
{
	assert(pPerson != 0);
	m_pPerson = pPerson;

	double tdebut = pPerson->getDebutTime();
	int Dp = pPerson->getNumberOfDiagnosedPartners();

	double A = baseline + diagpartnersfactor * Dp - beta * tdebut;
	double B = beta;

	setAB(A, B);
}

ConfigFunctions prepOfferedConfigFunctions(EventPrePOffered::processConfig, EventPrePOffered::obtainConfig, "EventPrePOffered");

JSONConfig prepOfferedJSONConfig(R"JSON(
        "EventPrePOffered": {
            "depends": null,
            "params": [
                [ "prepoffered.baseline", 0 ],
				[ "prepoffered.diagpartnersfactor", 0],
                [ "prepoffered.beta", 0 ],
                [ "prepoffered.t_max", 200 ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
