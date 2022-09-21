#include "eventsyphilisprogression.h"

#include "eventsyphilistransmission.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "person_syphilis.h"

using namespace std;

EventSyphilisProgression::EventSyphilisProgression(Person *pPerson) : SimpactEvent(pPerson) {}

EventSyphilisProgression::~EventSyphilisProgression() {}

string EventSyphilisProgression::getDescription(double tNow) const
{
	return strprintf("Syphilis progression event for %s", getPerson(0)->getName().c_str());
}

void EventSyphilisProgression::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "syphilis progression", tNow, pPerson, 0);
}

void EventSyphilisProgression::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	pPerson->syphilis().progress(t);

	Person_Syphilis::SyphilisDiseaseStage diseaseStage = pPerson->syphilis().getDiseaseStage();
	// Schedule new progression event if necessary.
	// I.e. schedule primary --> secondary, secondary --> latent, and latent --> secondary or latent --> tertiary
	if (diseaseStage == Person_Syphilis::Primary || diseaseStage == Person_Syphilis::Secondary || diseaseStage == Person_Syphilis::Latent) {
		EventSyphilisProgression *pEvtProgression = new EventSyphilisProgression(pPerson);
		population.onNewEvent(pEvtProgression);
	}

	// Schedule infection event to partners if person becomes infectious (i.e. enters primary stage)
	if (diseaseStage == Person_Syphilis::Primary) {
		// Check relationships person is in, and if partner not yet infected with syphilis, schedule transmission event
		// now that the person is infectious
		int numRelations = pPerson->getNumberOfRelationships();
		pPerson->startRelationshipIteration();

		for (int i = 0; i < numRelations; i++) {
			double formationTime = -1;
			Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);

			if (!pPartner->syphilis().isInfected()) {
				EventSyphilisTransmission *pEvtTrans = new EventSyphilisTransmission(pPerson, pPartner);
				population.onNewEvent(pEvtTrans);
			}
		}
	}
}

void EventSyphilisProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	delete s_pExposedStageDurationDistribution;
	s_pExposedStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "syphilisprogression.exposedstageduration");

	delete s_pPrimaryStageDurationDistribution;
	s_pPrimaryStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "syphilisprogression.primarystageduration");

	delete s_pSecondaryStageDurationDistribution;
	s_pSecondaryStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "syphilisprogression.secondarystageduration");

	delete s_pLatentStageDurationDistribution;
	s_pLatentStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "syphilisprogression.latentstageduration");
}

void EventSyphilisProgression::obtainConfig(ConfigWriter &config)
{
	assert(s_pExposedStageDurationDistribution);
	addDistributionToConfig(s_pExposedStageDurationDistribution, config, "syphilisprogression.exposedstageduration");

	assert(s_pPrimaryStageDurationDistribution);
	addDistributionToConfig(s_pPrimaryStageDurationDistribution, config, "syphilisprogression.primarystageduration");

	assert(s_pSecondaryStageDurationDistribution);
	addDistributionToConfig(s_pSecondaryStageDurationDistribution, config, "syphilisprogression.secondarystageduration");

	assert(s_pLatentStageDurationDistribution);
	addDistributionToConfig(s_pLatentStageDurationDistribution, config, "syphilisprogression.latentstageduration");
}

double EventSyphilisProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);

	double dt = 0;

	Person_Syphilis::SyphilisDiseaseStage diseaseStage = pPerson->syphilis().getDiseaseStage();
	if (diseaseStage == Person_Syphilis::Exposed) {
		dt = s_pExposedStageDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Syphilis::Primary) {
		dt = s_pPrimaryStageDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Syphilis::Secondary) {
		dt = s_pSecondaryStageDurationDistribution->pickNumber();
	} else if (diseaseStage == Person_Syphilis::Latent) {
		dt = s_pLatentStageDurationDistribution->pickNumber();
	}

	return dt; // TODO is there any reason NOT to calculate it like this?
	// (i.e. instead of tEvt = tInfection + duration of stages up till now --> dt = tEvt - population.getTime()
}

ProbabilityDistribution *EventSyphilisProgression::s_pExposedStageDurationDistribution = 0;
ProbabilityDistribution *EventSyphilisProgression::s_pPrimaryStageDurationDistribution = 0;
ProbabilityDistribution *EventSyphilisProgression::s_pSecondaryStageDurationDistribution = 0;
ProbabilityDistribution *EventSyphilisProgression::s_pLatentStageDurationDistribution = 0;

ConfigFunctions syphilisProgressionConfigFunctions(EventSyphilisProgression::processConfig, EventSyphilisProgression::obtainConfig, "EventSyphilisProgression");

JSONConfig syphilisProgressionJSONConfig(R"JSON(
        "EventSyphilisProgression": {
            "depends": null,
            "params": [
                [ "syphilisprogression.exposedstageduration.dist", "distTypes", [ "fixed", [ [ "value", 0.16 ] ] ] ],
				[ "syphilisprogression.primarystageduration.dist", "distTypes", [ "fixed", [ [ "value", 0.1 ] ] ] ],
				[ "syphilisprogression.secondarystageduration.dist", "distTypes", [ "fixed", [ [ "value", 1 ] ] ] ],
                [ "syphilisprogression.latentstageduration.dist", "distTypes", [ "fixed", [ [ "value", 3 ] ] ] ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
