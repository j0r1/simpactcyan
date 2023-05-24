#include "eventdiagnosis.h"
#include "configsettings.h"
#include "configwriter.h"
#include "eventmonitoring.h"
#include "eventprepoffered.h"
#include "configdistributionhelper.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>

using namespace std;

EventDiagnosis::EventDiagnosis(Person *pPerson, bool scheduleImmediately, bool seedingEvent) : SimpactEvent(pPerson), m_scheduleImmediately(scheduleImmediately), m_seedingEvent(seedingEvent)
{
}

EventDiagnosis::~EventDiagnosis()
{
}

string EventDiagnosis::getDescription(double tNow) const
{
	return strprintf("Diagnosis event for %s", getPerson(0)->getName().c_str());
}

void EventDiagnosis::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	if (m_seedingEvent) {
		writeEventLogStart(true, "diagnosis", tNow, pPerson, 0, true);
	} else {
		writeEventLogStart(true, "diagnosis", tNow, pPerson, 0);
	}
}

void EventDiagnosis::markOtherAffectedPeople(const PopulationStateInterface &population)
{
	Person *pPerson = getPerson(0);

	// Infected partners (who possibly have a diagnosis event, of which
	// the hazard depends on the number of diagnosed partners), are also
	// affected!
	int numRel = pPerson->getNumberOfRelationships();

	pPerson->startRelationshipIteration();
	for (int i = 0 ; i < numRel ; i++)
	{
		double tDummy;
		Person *pPartner = pPerson->getNextRelationshipPartner(tDummy);

		if (pPartner->hiv().isInfected())
			population.markAffectedPerson(pPartner);
	}

#ifndef NDEBUG
	// Double check that the iteration is done
	double tDummy;
	assert(pPerson->getNextRelationshipPartner(tDummy) == 0);
#endif // NDEBUG
}

void EventDiagnosis::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson->hiv().isInfected());
	assert(!pPerson->hiv().hasLoweredViralLoad());

	// Mark the person as diagnosed
	pPerson->hiv().increaseDiagnoseCount(t);

	// If on PreP, stop PreP
	if (pPerson->hiv().isOnPreP()) {
		pPerson->hiv().stopPreP();
	  writeEventLogStart(true, "prepstopped", t, pPerson, 0);
	  pPerson->writeToPrepLog(t, "HIV diagnosis");
	}

	// Update PreP eligibility of partners
	int numRelations = pPerson->getNumberOfRelationships();
	pPerson->startRelationshipIteration();
	
	for (int i = 0 ; i < numRelations ; i++)
	{
	  double formationTime = -1;
	  Person *pPartner = pPerson->getNextRelationshipPartner(t);
	  
	  if (!pPartner->hiv().isInfected())
	  {
	    bool schedulePrePOfferedEvent = pPartner->hiv().updatePrePEligibility(t);
	    if(schedulePrePOfferedEvent){
	      EventPrePOffered *pEvtPreP = new EventPrePOffered(pPartner, true);
	      population.onNewEvent(pEvtPreP);
	    }
	    
	  }
	}
	
	// Schedule an initial monitoring event right away! (the 'true' is for 'right away')
	EventMonitoring *pEvtMonitor = new EventMonitoring(pPerson, true);
	population.onNewEvent(pEvtMonitor);

}

double EventDiagnosis::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	// This is for the diagnosis event that should be scheduled right after the
	// screening event
	if (m_scheduleImmediately)
	{
		double minute = 1.0/(365.0*24.0*60.0); // a minute in a unit of a year
		return minute;
	}

	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionDiagnosis h0(pPerson, s_baseline, s_ageFactor, s_genderFactor, s_diagPartnersFactor, s_numPartnersFactor,
			           s_isDiagnosedFactor, s_healthSeekingPropensityFactor, s_beta, s_HSV2factor);
	TimeLimitedHazardFunction h(h0, tMax);

	double internalTimeInterval = h.calculateInternalTimeInterval(t0, dt);

	if (s_routineTestingEnabled) {
		double timeFromInfectionToTest = s_uniformDistribution.pickNumber();
		double timeOfTest = pPerson->hiv().getInfectionTime() + timeFromInfectionToTest;
		double timeUntilTest = timeOfTest - t0;

		if (timeUntilTest < internalTimeInterval) {
			return timeUntilTest;
		}
	}

	return internalTimeInterval;
}

double EventDiagnosis::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	// This is for the diagnosis event that should be scheduled right after the
	// screening event
	if (m_scheduleImmediately)
	{
		double minute = 1.0/(365.0*24.0*60.0); // a minute in a unit of a year
		return minute;
	}

	Person *pPerson = getPerson(0);
	double tMax = getTMax(pPerson);

	HazardFunctionDiagnosis h0(pPerson, s_baseline, s_ageFactor, s_genderFactor, s_diagPartnersFactor, s_numPartnersFactor,
			           s_isDiagnosedFactor, s_healthSeekingPropensityFactor, s_beta, s_HSV2factor);
	TimeLimitedHazardFunction h(h0, tMax);

	double realTimeInterval = h.solveForRealTimeInterval(t0, Tdiff);

	if (s_routineTestingEnabled) {
		double timeFromInfectionToTest = s_uniformDistribution.pickNumber();
		double timeOfTest = pPerson->hiv().getInfectionTime() + timeFromInfectionToTest;
		double timeUntilTest = timeOfTest - t0;

		if (timeUntilTest < realTimeInterval) {
			return timeUntilTest;
		}
	}

	return realTimeInterval;
}

double EventDiagnosis::getTMax(const Person *pPerson)
{
	assert(pPerson != 0);
	double tb = pPerson->getDateOfBirth();

	assert(s_tMax > 0);
	return tb + s_tMax;
}

double EventDiagnosis::s_baseline = 0;
double EventDiagnosis::s_ageFactor = 0;
double EventDiagnosis::s_genderFactor = 0;
double EventDiagnosis::s_diagPartnersFactor = 0;
double EventDiagnosis::s_numPartnersFactor = 0;
double EventDiagnosis::s_isDiagnosedFactor = 0;
double EventDiagnosis::s_healthSeekingPropensityFactor = 0;
double EventDiagnosis::s_beta = 0;
double EventDiagnosis::s_HSV2factor = 0;
double EventDiagnosis::s_tMax = 0;

bool EventDiagnosis::s_routineTestingEnabled = false;
double EventDiagnosis::s_routineTestingInterval = 0;
UniformDistribution EventDiagnosis::s_uniformDistribution = UniformDistribution(0, 1, 0);


void EventDiagnosis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("diagnosis.baseline", s_baseline)) ||
	    !(r = config.getKeyValue("diagnosis.agefactor", s_ageFactor)) ||
	    !(r = config.getKeyValue("diagnosis.genderfactor", s_genderFactor)) ||
	    !(r = config.getKeyValue("diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
		!(r = config.getKeyValue("diagnosis.numpartnersfactor", s_numPartnersFactor)) ||
	    !(r = config.getKeyValue("diagnosis.isdiagnosedfactor", s_isDiagnosedFactor)) ||
		!(r = config.getKeyValue("diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
	    !(r = config.getKeyValue("diagnosis.beta", s_beta)) ||
	    !(r = config.getKeyValue("diagnosis.t_max", s_tMax))||
	    !(r = config.getKeyValue("diagnosis.HSV2factor", s_HSV2factor)) ||
		!(r = config.getKeyValue("diagnosis.routinetesting.enabled", s_routineTestingEnabled)) ||
		!(r = config.getKeyValue("diagnosis.routinetesting.interval", s_routineTestingInterval))
	   )
		abortWithMessage(r.getErrorString());

	s_uniformDistribution = UniformDistribution(0, s_routineTestingInterval, pRndGen);
}

void EventDiagnosis::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("diagnosis.baseline", s_baseline)) ||
	    !(r = config.addKey("diagnosis.agefactor", s_ageFactor)) ||
	    !(r = config.addKey("diagnosis.genderfactor", s_genderFactor)) ||
	    !(r = config.addKey("diagnosis.diagpartnersfactor", s_diagPartnersFactor)) ||
		!(r = config.addKey("diagnosis.numpartnersfactor", s_numPartnersFactor)) ||
	    !(r = config.addKey("diagnosis.isdiagnosedfactor", s_isDiagnosedFactor)) ||
		!(r = config.addKey("diagnosis.healthseekingpropensityfactor", s_healthSeekingPropensityFactor)) ||
	    !(r = config.addKey("diagnosis.beta", s_beta)) ||
	    !(r = config.addKey("diagnosis.t_max", s_tMax)) ||
	    !(r = config.addKey("diagnosis.HSV2factor", s_HSV2factor)) ||
		!(r = config.addKey("diagnosis.routinetesting.enabled", s_routineTestingEnabled)) ||
		!(r = config.addKey("diagnosis.routinetesting.interval", s_routineTestingInterval))
	   )
		abortWithMessage(r.getErrorString());
}

// exp(baseline + ageFactor*(t-t_birth) + genderFactor*gender + diagPartnersFactor*numDiagnosedPartners
//     isDiagnosedFactor*hasBeenDiagnosed + healthSeekingPropensityFactor*healthSeekingPropensity + beta*(t-t_infected) + HSV2factor*HSV2)
//
// = exp(A + B*t) with
// 
//  A = baseline - ageFactor*t_birth + genderFactor*gender + diagPartnersFactor*numDiagnosedPartners
//  + numPartnersFactor*numRelationships + HSV2factor*HSV2
//      + isDiagnosedFactor*hasBeenDiagnosed + healthSeekingPropensityFactor*healthSeekingPropensity - beta*t_infected
//  B = ageFactor + beta
HazardFunctionDiagnosis::HazardFunctionDiagnosis(Person *pPerson, double baseline, double ageFactor,
		                                                 double genderFactor, double diagPartnersFactor, double numPartnersFactor,
							         double isDiagnosedFactor, double healthSeekingPropensityFactor, double beta, double HSV2factor)
	: m_baseline(baseline), m_ageFactor(ageFactor), m_genderFactor(genderFactor),
	  m_diagPartnersFactor(diagPartnersFactor), m_numPartnersFactor(numPartnersFactor), m_isDiagnosedFactor(isDiagnosedFactor), m_healthSeekingPropensityFactor(healthSeekingPropensityFactor),
	  m_beta(beta), m_HSV2factor(HSV2factor)
{
	assert(pPerson != 0);
	m_pPerson = pPerson;

	double tb = pPerson->getDateOfBirth();
	double tinf = pPerson->hiv().getInfectionTime();
	double G = (pPerson->isMan())?0:1;
	int D = pPerson->getNumberOfDiagnosedPartners();
	int P = pPerson->getNumberOfRelationships();
	int hasBeenDiagnosed = (pPerson->hiv().isDiagnosed())?1:0;
	double H = pPerson->getHealthSeekingPropensity();
	int HSV2 = (pPerson->hsv2().isInfected())?1:0;

	double A = baseline - ageFactor*tb + genderFactor*G + diagPartnersFactor*D + numPartnersFactor*P + isDiagnosedFactor*hasBeenDiagnosed + healthSeekingPropensityFactor*H - beta*tinf + HSV2factor*HSV2;
	double B = ageFactor + beta;

	setAB(A, B);
}

// This implementation is not necessary for running, it is provided for testing purposes
double HazardFunctionDiagnosis::evaluate(double t)
{
	double tb = m_pPerson->getDateOfBirth();
	double tinf = m_pPerson->hiv().getInfectionTime();
	double G = (m_pPerson->isMan())?0:1;
	int D = m_pPerson->getNumberOfDiagnosedPartners();
	int P = m_pPerson->getNumberOfRelationships();
	int hasBeenDiagnosed = (m_pPerson->hiv().isDiagnosed())?1:0;
	double H = m_pPerson->getHealthSeekingPropensity();
	int HSV2 = (m_pPerson->hsv2().isInfected()) ?1:0;

	double age = (t-tb);

	return std::exp(m_baseline + m_ageFactor*age + m_genderFactor*G + m_diagPartnersFactor*D + m_numPartnersFactor*P +
			m_isDiagnosedFactor*hasBeenDiagnosed + m_healthSeekingPropensityFactor*H + m_beta*(t-tinf)+ m_HSV2factor*HSV2);
}

ConfigFunctions diagnosisConfigFunctions(EventDiagnosis::processConfig, EventDiagnosis::obtainConfig, "EventDiagnosis");

JSONConfig diagnosisJSONConfig(R"JSON(
        "EventDiagnosis": {
            "depends": null,
            "params": [
                [ "diagnosis.baseline", 0 ],
                [ "diagnosis.agefactor", 0 ],
                [ "diagnosis.genderfactor", 0 ],
                [ "diagnosis.diagpartnersfactor", 0 ],
				[ "diagnosis.numpartnersfactor", 0 ],
                [ "diagnosis.isdiagnosedfactor", 0 ],
                [ "diagnosis.beta", 0 ],
				[ "diagnosis.healthseekingpropensityfactor", 0 ],
		     	[ "diagnosis.HSV2factor", 0 ],
                [ "diagnosis.t_max", 200 ],
				[ "diagnosis.routinetesting.enabled", "no"],
				[ "diagnosis.routinetesting.interval", 10]	
            ],
            "info": [
                "When a person gets infected or drops out of treatment, a diagnosis event is ",
                "scheduled of which the fire time is determined by the following hazard:",
                "",
                " h = exp(baseline + agefactor*A(t) + genderfactor*G ",
                "         + diagpartnersfactor*ND + numpartnersfactor*P + isdiagnosedfactor*D",
                "         + beta*t + HSV2factor*HSV2)",
                "",
                "Here, A(t) is the age of the person, G is the gender (0 for a man, 1 for a",
                "woman), ND is the number of diagnosed partners, P is the number of partners and D is a flag (0 or 1)",
                "indicating if the person has been on treatment before (to have different",
                "behaviour for first diagnosis and re-testing after dropout)",
				"and H is the health-seeking propensity of the person."
            ]
        })JSON");
