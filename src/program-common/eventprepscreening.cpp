#include "eventprepscreening.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventdiagnosis.h"
#include "eventchlamydiadiagnosis.h"
#include "eventgonorrheadiagnosis.h"
#include "eventsyphilisdiagnosis.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "probabilitydistribution.h"

using namespace std;

EventPrePScreening::EventPrePScreening(Person *pPerson, bool scheduleImmediately): SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately) {}

EventPrePScreening::~EventPrePScreening() {}

string EventPrePScreening::getDescription(double tNow) const
{
	return strprintf("PreP screening event for %s", getPerson(0)->getName().c_str());
}

void EventPrePScreening::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "prepscreening", tNow, pPerson, 0);
}

void EventPrePScreening::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	// if HIV infected, and not yet diagnosed, immediately schedule diagnosis
	if (pPerson->hiv().isInfected() && (!pPerson->hiv().isDiagnosed())) {
		EventDiagnosis *pEvtDiagnosis = new EventDiagnosis(pPerson, true);
		population.onNewEvent(pEvtDiagnosis); // stopPrEP() is done in diagnosis event
	}

	// if STI infected, immediate diagnosis + treatment
	if(pPerson->chlamydia().isInfected()){
	  EventChlamydiaDiagnosis *pEvtChlamydiaDiagnosis = new EventChlamydiaDiagnosis(pPerson, true);
	  population.onNewEvent(pEvtChlamydiaDiagnosis);
	  // pPerson->chlamydia().diagnose(t);
	  // writeEventLogStart(true, "chlamydia diagnosis", t, pPerson, 0);
	  // pPerson->chlamydia().progress(t, true);
	  // writeEventLogStart(true, "(chlamydia treatment)", t, pPerson, 0);
	 }
	
	if(pPerson->gonorrhea().isInfected()){
	  EventGonorrheaDiagnosis *pEvtGonorrheaDiagnosis = new EventGonorrheaDiagnosis(pPerson, true);
	  population.onNewEvent(pEvtGonorrheaDiagnosis);
	  // pPerson->gonorrhea().diagnose(t);
	  // writeEventLogStart(true, "gonorrhea diagnosis", t, pPerson, 0);
	  // pPerson->gonorrhea().progress(t, true);
	  // writeEventLogStart(true, "(gonorrhea treatment)", t, pPerson, 0);
	}
	
	if(pPerson->syphilis().isInfected()){
	  EventSyphilisDiagnosis *pEvtSyphilisDiagnosis = new EventSyphilisDiagnosis(pPerson, true);
	  population.onNewEvent(pEvtSyphilisDiagnosis);
	  // pPerson->syphilis().diagnose(t);
	  // writeEventLogStart(true, "syphilis diagnosis", t, pPerson, 0);
	  // pPerson->syphilis().progress(t, true);
	  // writeEventLogStart(true, "(syphilis treatment)", t, pPerson, 0);
	}
	

	// If not HIV infected, schedule new screening
	if (!pPerson->hiv().isInfected()) {
		EventPrePScreening *pNewScreening = new EventPrePScreening(pPerson);
		population.onNewEvent(pNewScreening);
	}
}

void EventPrePScreening::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	delete s_pScreeningIntervalDistribution;
	s_pScreeningIntervalDistribution = getDistributionFromConfig(config, pRndGen, "prepscreening.interval");
}

void EventPrePScreening::obtainConfig(ConfigWriter &config)
{
	assert(s_pScreeningIntervalDistribution);
	addDistributionToConfig(s_pScreeningIntervalDistribution, config, "prepscreening.interval");
}

bool EventPrePScreening::isUseless(const PopulationStateInterface &pop)
{
	// PreP screening event becomes useless if person has been diagnosed with HIV
	Person *pPerson = getPerson(0);

	if (pPerson->hiv().isDiagnosed()) {
		return true;
	}
	
	// also useless if person no longer on PrEP (e.g. if eligibility changed and PrEP was stopped)
	if (!(pPerson->hiv().isOnPreP())){
	  return true;
	}

	return false;
}

double EventPrePScreening::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	// This is for the screening event that should be scheduled right after the
	// prep start event event
	if (m_scheduleImmediately)
	{
		double minute = 1.0/(365.0*24.0*60.0); // a minute in a unit of a year
		return minute;
	}

	double dt = s_pScreeningIntervalDistribution->pickNumber();

	assert(dt >= 0);

	return dt;
}

ProbabilityDistribution *EventPrePScreening::s_pScreeningIntervalDistribution = 0;

ConfigFunctions prepScreeningConfigFunctions(EventPrePScreening::processConfig, EventPrePScreening::obtainConfig, "EventPrePScreening");

JSONConfig prepScreeningJSONConfig(R"JSON(
        "EventPrePScreening": {
            "depends": null,
            "params": [ ["prepscreening.interval.dist", "distTypes", ["fixed", [ [ "value", 0.25 ] ] ] ] ],
            "info": [
                "TODO"
            ]
        })JSON");
