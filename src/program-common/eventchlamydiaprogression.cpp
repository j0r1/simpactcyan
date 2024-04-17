#include "eventchlamydiaprogression.h"
#include "eventchlamydiatransmission.h"
#include "eventchlamydiadiagnosis.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "jsonconfig.h"
#include "configfunctions.h"
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

	pPerson->chlamydia().progress(t, false); // natural clearance or progression from exposed to symptomatic/asymptomatic stage
	pPerson->writeToChlamydiaTreatLog();

	// Schedule new progression event if necessary.
	// I.e. schedule recovery event if person just progressed to symptomatic or asymptomatic
	Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
	if (diseaseStage == Person_Chlamydia::Symptomatic || diseaseStage == Person_Chlamydia::Asymptomatic)// || diseaseStage == Person_Chlamydia::Immune)
	{
		EventChlamydiaProgression *pEvtProgression = new EventChlamydiaProgression(pPerson);
		population.onNewEvent(pEvtProgression);
		
		// also schedule diagnosis for symptomatic persons
		if(diseaseStage == Person_Chlamydia::Symptomatic){
		  EventChlamydiaDiagnosis *pEvtDiag = new EventChlamydiaDiagnosis(pPerson, false);
		  population.onNewEvent(pEvtDiag);
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

bool EventChlamydiaProgression::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if person no longer infected due to treatment
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->chlamydia().isInfected()){
    return true;
  }
  
  return false;
}


void EventChlamydiaProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

	// delete s_pExposedStageDurationDistribution;
	// s_pExposedStageDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.exposedstageduration");
	// 
	// delete s_pSymptomaticInfectionDurationDistribution;
	// s_pSymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.symptomaticinfectionduration");
	// 
	// delete s_pAsymptomaticInfectionDurationDistribution;
	// s_pAsymptomaticInfectionDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.asymptomaticinfectionduration");
	// 
	// delete s_pImmunityDurationDistribution;
	// s_pImmunityDurationDistribution = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.immunityduration");
	
	// rectal
	delete s_pInfectionDurationDistributionRectal;
	s_pInfectionDurationDistributionRectal = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.infectionduration.rectal");
	
	// urethral
	delete s_pInfectionDurationDistributionUrethral;
	s_pInfectionDurationDistributionUrethral = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.infectionduration.urethral");
	
	// vaginal
	delete s_pInfectionDurationDistributionVaginal;
	s_pInfectionDurationDistributionVaginal = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.infectionduration.vaginal");
	
	// time to sypmtoms
	delete s_pTimeToSymptoms;
	s_pTimeToSymptoms = getDistributionFromConfig(config, pRndGen, "chlamydiaprogression.timetosymptoms");
	
}

void EventChlamydiaProgression::obtainConfig(ConfigWriter &config)
{
	// assert(s_pExposedStageDurationDistribution);
	// addDistributionToConfig(s_pExposedStageDurationDistribution, config, "chlamydiaprogression.exposedstageduration");
	// 
	// assert(s_pSymptomaticInfectionDurationDistribution);
	// addDistributionToConfig(s_pSymptomaticInfectionDurationDistribution, config, "chlamydiaprogression.symptomaticinfectionduration");
	// 
	// assert(s_pAsymptomaticInfectionDurationDistribution);
	// addDistributionToConfig(s_pAsymptomaticInfectionDurationDistribution, config, "chlamydiaprogression.asymptomaticinfectionduration");
	// 
	// assert(s_pImmunityDurationDistribution);
	// addDistributionToConfig(s_pImmunityDurationDistribution, config, "chlamydiaprogression.immunityduration");
	
	assert(s_pInfectionDurationDistributionRectal);
  addDistributionToConfig(s_pInfectionDurationDistributionRectal, config, "chlamydiaprogression.infectionduration.rectal");
  
  assert(s_pInfectionDurationDistributionUrethral);
  addDistributionToConfig(s_pInfectionDurationDistributionUrethral, config, "chlamydiaprogression.infectionduration.urethral");
  
  assert(s_pInfectionDurationDistributionVaginal);
  addDistributionToConfig(s_pInfectionDurationDistributionVaginal, config, "chlamydiaprogression.infectionduration.vaginal");
  
  assert(s_pTimeToSymptoms);
  addDistributionToConfig(s_pTimeToSymptoms, config, "chlamydiaprogression.timetosymptoms");

}

double EventChlamydiaProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);
	assert(pPerson->chlamydia().isInfected());
	
	Person_Chlamydia::InfectionSite infectionSite = pPerson->chlamydia().getInfectionSite();
	Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
	double dt = 0;
	
	// Natural clearance rates
	if (diseaseStage == Person_Chlamydia::Symptomatic || diseaseStage == Person_Chlamydia::Asymptomatic){
	  if (infectionSite == Person_Chlamydia::Rectal) {
	    dt = s_pInfectionDurationDistributionRectal->pickNumber();
	  }
	  else if (infectionSite == Person_Chlamydia::Urethral) {
	    dt = s_pInfectionDurationDistributionUrethral->pickNumber();
	  }
	  else if (infectionSite == Person_Chlamydia::Vaginal) {
	    dt = s_pInfectionDurationDistributionVaginal->pickNumber();
	  }	  
	// Time to symptoms for exposed individuals
	} else if (diseaseStage == Person_Chlamydia::Exposed){
	  dt = s_pTimeToSymptoms->pickNumber();
	}
	


	// Person_Chlamydia::ChlamydiaDiseaseStage diseaseStage = pPerson->chlamydia().getDiseaseStage();
	// if (diseaseStage == Person_Chlamydia::Exposed) {
	// 	dt = s_pExposedStageDurationDistribution->pickNumber();
	// } else if (diseaseStage == Person_Chlamydia::Symptomatic) {
	// 	dt = s_pSymptomaticInfectionDurationDistribution->pickNumber();
	// } else if (diseaseStage == Person_Chlamydia::Asymptomatic) {
	// 	dt = s_pAsymptomaticInfectionDurationDistribution->pickNumber();
	// } else if (diseaseStage == Person_Chlamydia::Immune) {
	// 	dt = s_pImmunityDurationDistribution->pickNumber();
	// }

	assert(dt >= 0);
	
	return dt; // TODO is there any reason NOT to calculate it like this?
	// (i.e. instead of tEvt = tInfection + duration of stages up till now --> dt = tEvt - population.getTime()
}

// ProbabilityDistribution *EventChlamydiaProgression::s_pExposedStageDurationDistribution = 0;
// ProbabilityDistribution *EventChlamydiaProgression::s_pSymptomaticInfectionDurationDistribution = 0;
// ProbabilityDistribution *EventChlamydiaProgression::s_pAsymptomaticInfectionDurationDistribution = 0;
// ProbabilityDistribution *EventChlamydiaProgression::s_pImmunityDurationDistribution = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pInfectionDurationDistributionRectal = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pInfectionDurationDistributionUrethral = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pInfectionDurationDistributionVaginal = 0;
ProbabilityDistribution *EventChlamydiaProgression::s_pTimeToSymptoms = 0;

ConfigFunctions chlamydiaProgressionConfigFunctions(EventChlamydiaProgression::processConfig, EventChlamydiaProgression::obtainConfig, "EventChlamydiaProgression");

JSONConfig chlamydiaProgressionJSONConfig(R"JSON(
        "EventChlamydiaProgression": {
            "depends": null,
            "params": [
  [ "chlamydiaprogression.infectionduration.rectal.dist", "distTypes", [ "fixed", [ [ "value", 2.85 ] ] ] ],
  [ "chlamydiaprogression.infectionduration.urethral.dist", "distTypes", [ "fixed", [ [ "value", 2.85 ] ] ] ],
  [ "chlamydiaprogression.infectionduration.vaginal.dist", "distTypes", [ "fixed", [ [ "value", 1.35 ] ] ] ],
  [ "chlamydiaprogression.timetosymptoms.dist", "distTypes", [ "fixed", [ [ "value", 0.002 ] ] ] ]
            ],
            "info": [
                "TODO"
            ]
        })JSON");
