#include "eventgonorrheaprogression.h"

#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "person_gonorrhea.h"

using namespace std;

EventGonorrheaProgression::EventGonorrheaProgression(Person *pPerson) : SimpactEvent(pPerson) {}

EventGonorrheaProgression::~EventGonorrheaProgression() {}

string EventGonorrheaProgression::getDescription(double tNow) const
{
	return strprintf("Gonorrhea progression event for %s", getPerson(0)->getName().c_str());
}

void EventGonorrheaProgression::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "gonorrhea progression", tNow, pPerson, 0);
}

void EventGonorrheaProgression::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	pPerson->gonorrhea().progress(t);
}

void EventGonorrheaProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	delete s_pSymptomaticInfectionDurationDistribution;
	s_pSymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.symptomaticinfectionduration");

	delete s_pAsymptomaticInfectionDurationDistribution;
	s_pAsymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.asymptomaticinfectionduration");

}

void EventGonorrheaProgression::obtainConfig(ConfigWriter &config)
{
	assert(s_pSymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pSymptomaticInfectionDurationDistribution, config, "gonorrheaprogression.symptomaticinfectionduration");

	assert(s_pAsymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pAsymptomaticInfectionDurationDistribution, config, "gonorrheaprogression.asymptomaticinfectionduration");
}


double EventGonorrheaProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);

	double dt = 0;

	Person_Gonorrhea::GonorrheaDiseaseStage diseaseStage = pPerson->gonorrhea().getDiseaseStage();
	if (diseaseStage == Person_Gonorrhea::Symptomatic) {
		dt = s_pSymptomaticInfectionDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Gonorrhea::Asymptomatic) {
		dt = s_pAsymptomaticInfectionDurationDistribution->pickNumber();
	}

	return dt; // TODO is there any reason NOT to calculate it like this?
	// (i.e. instead of tEvt = tInfection + duration of stages up till now --> dt = tEvt - population.getTime()
}

ProbabilityDistribution *EventGonorrheaProgression::s_pSymptomaticInfectionDurationDistribution = 0;
ProbabilityDistribution *EventGonorrheaProgression::s_pAsymptomaticInfectionDurationDistribution = 0;


ConfigFunctions gonorrheaProgressionConfigFunctions(EventGonorrheaProgression::processConfig, EventGonorrheaProgression::obtainConfig, "EventGonorrheaProgression");

JSONConfig gonorrheaProgressionJSONConfig(R"JSON(
        "EventGonorrheaProgression": {
            "depends": null,
            "params": [
                [ "gonorrheaprogression.symptomaticinfectionduration.dist", "distTypes", [ "fixed", [ [ "value", 1 ] ] ] ],
                [ "gonorrheaprogression.asymptomaticinfectionduration.dist", "distTypes", [ "fixed", [ [ "value", 0.5 ] ] ] ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
