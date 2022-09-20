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

	assert(pPerson->gonorrhea().isInfected());

	// Mark the person as recovered from gonorrhea
	pPerson->gonorrhea().setRecovered(t);
}

void EventGonorrheaProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	delete s_pSymptomaticInfectionDurationDistribution;
	s_pSymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "eventgonorrheaprogression.symptomaticinfectionduration");

	delete s_pAsymptomaticInfectionDurationDistribution;
	s_pAsymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "eventgonorrheaprogression.asymptomaticinfectionduration");

}

void EventGonorrheaProgression::obtainConfig(ConfigWriter &config)
{
	assert(s_pSymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pSymptomaticInfectionDurationDistribution, config, "eventgonorrheaprogression.symptomaticinfectionduration");

	assert(s_pAsymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pAsymptomaticInfectionDurationDistribution, config, "eventgonorrheaprogression.asymptomaticinfectionduration");
}


double EventGonorrheaProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);

	double infectionDuration = 0;

	Person_Gonorrhea::GonorrheaDiseaseStage diseaseStage = pPerson->gonorrhea().getDiseaseStage();
	if (diseaseStage == Person_Gonorrhea::Symptomatic) {
		infectionDuration = s_pSymptomaticInfectionDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Gonorrhea::Asymptomatic) {
		infectionDuration = s_pAsymptomaticInfectionDurationDistribution->pickNumber();
	}

	double tEvt = pPerson->gonorrhea().getInfectionTime() + infectionDuration;
	double dt = tEvt - population.getTime();

	assert(dt >= 0); // should not be in the past!

	return dt;
}

ProbabilityDistribution *EventGonorrheaProgression::s_pSymptomaticInfectionDurationDistribution = 0;
ProbabilityDistribution *EventGonorrheaProgression::s_pAsymptomaticInfectionDurationDistribution = 0;


ConfigFunctions gonorrheaProgressionConfigFunctions(EventGonorrheaProgression::processConfig, EventGonorrheaProgression::obtainConfig, "EventGonorrheaProgression");

JSONConfig gonorrheaProgressionJSONConfig(R"JSON(
        "EventProgressionRecovery": {
            "depends": null,
            "params": [
                [ "gonorrheaprogression.symptomaticinfectionduration.dist", "distTypes", [ "fixed", 1 ] ],
                [ "gonorrheaprogression.asymptomaticinfectionduration.dist", "distTypes", [ "fixed", 0.5 ] ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
