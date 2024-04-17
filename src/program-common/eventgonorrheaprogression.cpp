#include "eventgonorrheaprogression.h"
#include "eventgonorrheatransmission.h"
#include "eventgonorrheadiagnosis.h"
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
	
	pPerson->gonorrhea().progress(t, false); // 0 = natural clearance
	pPerson->writeToGonorrheaTreatLog();
	
	// Schedule new progression event if necessary.
	// I.e. schedule recovery event if person just progressed to symptomatic or asymptomatic
	Person_Gonorrhea::GonorrheaDiseaseStage diseaseStage = pPerson->gonorrhea().getDiseaseStage();
	if (diseaseStage == Person_Gonorrhea::Symptomatic || diseaseStage == Person_Gonorrhea::Asymptomatic)// || diseaseStage == Person_Chlamydia::Immune)
	{
	  EventGonorrheaProgression *pEvtProgression = new EventGonorrheaProgression(pPerson);
	  population.onNewEvent(pEvtProgression);
	  
	  // also schedule diagnosis for symptomatic persons
	  if(diseaseStage == Person_Gonorrhea::Symptomatic){
	    EventGonorrheaDiagnosis *pEvtDiag = new EventGonorrheaDiagnosis(pPerson, false);
	    population.onNewEvent(pEvtDiag);
	  }
	}
	
	// Schedule new transmission event from infectious partners if person becomes susceptible again
	if (diseaseStage == Person_Gonorrhea::Susceptible) {
	  int numRelations = pPerson->getNumberOfRelationships();
	  pPerson->startRelationshipIteration();
	  
	  for (int i = 0; i < numRelations; i++)
	  {
	    double formationTime = -1;
	    Person *pPartner = pPerson->getNextRelationshipPartner(formationTime);
	    if (pPartner->gonorrhea().isInfectious())
	    {
	      EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pPartner, pPerson);
	      population.onNewEvent(pEvtTrans);
	    }
	  }
	}
	
}

bool EventGonorrheaProgression::isUseless(const PopulationStateInterface&population)
{
  // Event becomes useless if person no longer infected due to treatment
  Person *pPerson1 = getPerson(0);
  if(!pPerson1->gonorrhea().isInfected()){
    return true;
  }
  
  return false;
}

void EventGonorrheaProgression::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);

  // rectal
	delete s_pInfectionDurationDistributionRectal;
	s_pInfectionDurationDistributionRectal = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.infectionduration.rectal");
	
	// urethral
	delete s_pInfectionDurationDistributionUrethral;
	s_pInfectionDurationDistributionUrethral = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.infectionduration.urethral");
	
	// vaginal
	delete s_pInfectionDurationDistributionVaginal;
	s_pInfectionDurationDistributionVaginal = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.infectionduration.vaginal");
	
	// time to sypmtoms
	delete s_pTimeToSymptoms;
	s_pTimeToSymptoms = getDistributionFromConfig(config, pRndGen, "gonorrheaprogression.timetosymptoms");

}

void EventGonorrheaProgression::obtainConfig(ConfigWriter &config)
{
	assert(s_pInfectionDurationDistributionRectal);
	addDistributionToConfig(s_pInfectionDurationDistributionRectal, config, "gonorrheaprogression.infectionduration.rectal");
	
	assert(s_pInfectionDurationDistributionUrethral);
	addDistributionToConfig(s_pInfectionDurationDistributionUrethral, config, "gonorrheaprogression.infectionduration.urethral");
	
	assert(s_pInfectionDurationDistributionVaginal);
	addDistributionToConfig(s_pInfectionDurationDistributionVaginal, config, "gonorrheaprogression.infectionduration.vaginal");
	
	assert(s_pTimeToSymptoms);
	addDistributionToConfig(s_pTimeToSymptoms, config, "gonorrheaprogression.timetosymptoms");

}


double EventGonorrheaProgression::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);
	assert(pPerson->gonorrhea().isInfected());

	Person_Gonorrhea::InfectionSite infectionSite = pPerson->gonorrhea().getInfectionSite();
	Person_Gonorrhea::GonorrheaDiseaseStage diseaseStage = pPerson->gonorrhea().getDiseaseStage();
	
	double dt = 0;

	if (diseaseStage == Person_Gonorrhea::Symptomatic || diseaseStage == Person_Gonorrhea::Asymptomatic){
	  if (infectionSite == Person_Gonorrhea::Rectal) {
	    dt = s_pInfectionDurationDistributionRectal->pickNumber();
	  }
	  else if (infectionSite == Person_Gonorrhea::Urethral) {
	    dt = s_pInfectionDurationDistributionUrethral->pickNumber();
	  }
	  else if (infectionSite == Person_Gonorrhea::Vaginal) {
	    dt = s_pInfectionDurationDistributionVaginal->pickNumber();
	  }
	}else if (diseaseStage == Person_Gonorrhea::Exposed){
	  dt = s_pTimeToSymptoms->pickNumber();
	}


	
	assert(dt >= 0); // should not be in the past!
	
	// return dt; // TODO is there any reason NOT to calculate it like this?
	return dt; 
	// (i.e. instead of tEvt = tInfection + duration of stages up till now --> dt = tEvt - population.getTime()
}

ProbabilityDistribution *EventGonorrheaProgression::s_pInfectionDurationDistributionRectal = 0;
ProbabilityDistribution *EventGonorrheaProgression::s_pInfectionDurationDistributionUrethral = 0;
ProbabilityDistribution *EventGonorrheaProgression::s_pInfectionDurationDistributionVaginal = 0;
ProbabilityDistribution *EventGonorrheaProgression::s_pTimeToSymptoms = 0;


ConfigFunctions gonorrheaProgressionConfigFunctions(EventGonorrheaProgression::processConfig, EventGonorrheaProgression::obtainConfig, "EventGonorrheaProgression");

JSONConfig gonorrheaProgressionJSONConfig(R"JSON(
        "EventGonorrheaProgression": {
            "depends": null,
            "params": [
                [ "gonorrheaprogression.infectionduration.rectal.dist", "distTypes", [ "fixed", [ [ "value", 0.7 ] ] ] ],
                [ "gonorrheaprogression.infectionduration.urethral.dist", "distTypes", [ "fixed", [ [ "value", 0.4 ] ] ] ],
                [ "gonorrheaprogression.infectionduration.vaginal.dist", "distTypes", [ "fixed", [ [ "value", 0.5 ] ] ] ],
                [ "gonorrheaprogression.timetosymptoms.dist", "distTypes", [ "fixed", [ [ "value", 0.002 ] ] ] ]
              ],
            "info": [
                "TODO"
            ]
        })JSON");
