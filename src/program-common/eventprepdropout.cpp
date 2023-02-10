#include "eventprepdropout.h"
#include "eventprepoffered.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"

using namespace std;

EventPrePDropout::EventPrePDropout(Person *pPerson): SimpactEvent(pPerson) {}

EventPrePDropout::~EventPrePDropout() {}

string EventPrePDropout::getDescription(double tNow) const
{
	return strprintf("PreP dropout event for %s", getPerson(0)->getName().c_str());
}

void EventPrePDropout::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "prepdropout", tNow, pPerson, 0);
}

void EventPrePDropout::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	// Stop PreP
	pPerson->hiv().stopPreP();

	// If still eligible for PreP, schedule new EventPrePOffered
	if (pPerson->hiv().isEligibleForPreP()) {
		EventPrePOffered *pEvtPrep = new EventPrePOffered(pPerson);
		population.onNewEvent(pEvtPrep);
	}
}

double EventPrePDropout::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	// TODO: this is just a temporaty solution, until a real hazard has been defined

	int count = 0;
	int maxCount = 1024;
	double dt = -1;

	assert(s_pPrePDropoutDistribution);

	while (dt < 0 && count++ < maxCount)
		dt = s_pPrePDropoutDistribution->pickNumber();

	if (dt < 0)
		abortWithMessage("EventPrePDropout: couldn't find a positive time interval for next event");

	return dt;
}

double EventPrePDropout::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// TODO: this will need to change when using a real hazard
	return dt;
}

double EventPrePDropout::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// TODO: this will need to change when using a real hazard
	return Tdiff;
}

ProbabilityDistribution *EventPrePDropout::s_pPrePDropoutDistribution = 0;


void EventPrePDropout::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	delete s_pPrePDropoutDistribution;
	s_pPrePDropoutDistribution = getDistributionFromConfig(config, pRndGen, "prepdropout.interval");
}

void EventPrePDropout::obtainConfig(ConfigWriter &config)
{
	addDistributionToConfig(s_pPrePDropoutDistribution, config, "prepdropout.interval");
}

ConfigFunctions prepdropoutConfigFunctions(EventPrePDropout::processConfig, EventPrePDropout::obtainConfig, "EventPrePDropout");

JSONConfig prepdropoutJSONConfig(R"JSON(
        "EventPrePDropout_Timing": {
            "depends": null,
            "params": [
                [ "prepdropout.interval.dist", "distTypes", [ "uniform", [ [ "min", 0.25  ], [ "max", 10.0 ] ] ] ]
            ],
            "info": [
                "Distribution to schedule PreP dropout events."
            ]
        })JSON");

