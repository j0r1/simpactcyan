#include "eventchlamydiaprogression.h"

#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventchlamydiatransmission.h"
#include "jsonconfig.h"
#include "person_chlamydia.h"

using namespace std;

EventChlamydiaProgression::EventChlamydiaProgression(Person *pPerson) : SimpactEvent(pPerson) {}

EventChlamydiaProgression::~EventChlamydiaProgression() {}

string EventChlamydiaProgression::getDescription(double tNow) const
{
	return strprintf("Chlamydia progression event for %s", getPerson(0)->getName().c_str());
}

void EventChlamydiaProgression::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "chlamydia progression", tNow, pPerson, 0);
}

void EventChlamydiaProgression::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	pPerson->chlamydia().progress(t);

	// Schedule new progression event if necessary.
	// I.e. schedule recovery event if person just progressed to symptomatic or asymptomatic,
	// or schedule end of immunity if progressed to immune.
	Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
	if (diseaseStage == Person_Chlamydia::Symptomatic || diseaseStage == Person_Chlamydia::Asymptomatic || diseaseStage == Person_Chlamydia::Immune)
	{
		EventChlamydiaProgression *pEvtProgression = new EventChlamydiaProgression(pPerson);
		population.onNewEvent(pEvtProgression);
	}

	if (diseaseStage == Person_Chlamydia::Symptomatic || diseaseStage == Person_Chlamydia::Asymptomatic) {
		// Check relationships pTarget is in, and if partner not yet infected with chlamydia, schedule transmission event now that person is infectious.
		int numRelations = pPerson->getNumberOfRelationships();
		pPerson->startRelationshipIteration();

		for (int i = 0; i < numRelations; i++)
		{
			double formationTime = -1;
			Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);

			if (!pPartner->chlamydia().isInfected())
			{
				EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pPerson, pPartner);
				population.onNewEvent(pEvtTrans);
			}
		}
	}

	// Schedule new transmission event from infectious partners if person becomes susceptible again
	if (diseaseStage == Person_Chlamydia::Susceptible) {
		int numRelations = pPerson->getNumberOfRelationships();
		pPerson->startRelationshipIteration();

		for (int i = 0; i < numRelations; i++)
		{
			double formationTime = -1;
			Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
			if (pPartner->chlamydia().isInfectious())
			{
				EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pPartner, pPerson);
				population.onNewEvent(pEvtTrans);
			}
		}
	}
}

void EventChlamydiaProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	delete s_pExposedStageDurationDistribution;
	s_pExposedStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.exposedstageduration");

	delete s_pSymptomaticInfectionDurationDistribution;
	s_pSymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.symptomaticinfectionduration");

	delete s_pAsymptomaticInfectionDurationDistribution;
	s_pAsymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.asymptomaticinfectionduration");

	delete s_pImmunityDurationDistribution;
	s_pImmunityDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.immunityduration");
}

void EventChlamydiaProgression::obtainConfig(ConfigWriter &config)
{
	assert(s_pExposedStageDurationDistribution);
	addDistributionToConfig(s_pExposedStageDurationDistribution, config, "chlamydiaprogression.exposedstageduration");

	assert(s_pSymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pSymptomaticInfectionDurationDistribution, config, "chlamydiaprogression.symptomaticinfectionduration");

	assert(s_pAsymptomaticInfectionDurationDistribution);
	addDistributionToConfig(s_pAsymptomaticInfectionDurationDistribution, config, "chlamydiaprogression.asymptomaticinfectionduration");

	assert(s_pImmunityDurationDistribution);
	addDistributionToConfig(s_pImmunityDurationDistribution, config, "chlamydiaprogression.immunityduration");

}

double EventChlamydiaProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);

	double dt = 0;

	Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
	if (diseaseStage == Person_Chlamydia::Exposed) {
		dt = s_pExposedStageDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Chlamydia::Symptomatic) {
		dt = s_pSymptomaticInfectionDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Chlamydia::Asymptomatic) {
		dt = s_pAsymptomaticInfectionDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Chlamydia::Immune) {
		dt = s_pImmunityDurationDistribution->pickNumber();
	}

	return dt; // TODO is there any reason NOT to calculate it like this?
	// (i.e. instead of tEvt = tInfection + duration of stages up till now --> dt = tEvt - population.getTime()
}

ProbabilityDistribution *EventChlamydiaProgression::s_pExposedStageDurationDistribution = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pSymptomaticInfectionDurationDistribution = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pAsymptomaticInfectionDurationDistribution = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pImmunityDurationDistribution = 0;

ConfigFunctions chlamydiaProgressionConfigFunctions(EventChlamydiaProgression::processConfig, EventChlamydiaProgression::obtainConfig, "EventChlamydiaProgression");

JSONConfig chlamydiaProgressionJSONConfig(R"JSON(
        "EventChlamydiaProgression": {
            "depends": null,
            "params": [
				[ "chlamydiaprogression.exposedstageduration.dist", "distTypes", [ "fixed", [ [ "value", 0 ] ] ] ],
				[ "chlamydiaprogression.symptomaticinfectionduration.dist", "distTypes", [ "fixed", [ [ "value", 0.1 ] ] ] ],
				[ "chlamydiaprogression.asymptomaticinfectionduration.dist", "distTypes", [ "fixed", [ [ "value", 1 ] ] ] ],
                [ "chlamydiaprogression.immunityduration.dist", "distTypes", [ "fixed", [ [ "value", 0.5 ] ] ] ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
