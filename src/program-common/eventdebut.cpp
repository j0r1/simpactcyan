#include "eventdebut.h"
#include "eventformation.h"
#include "eventroutinetesting.h"
#include "gslrandomnumbergenerator.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <iostream>
#include "eventprepoffered.h"

EventDebut::EventDebut(Person *pPerson) : SimpactEvent(pPerson)
{
}

EventDebut::~EventDebut()
{
}

double EventDebut::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson = getPerson(0);

	assert(pPerson != 0);
	assert(getNumberOfPersons() == 1);

	assert(m_debutAge > 0);

	double tEvt = pPerson->getDateOfBirth() + m_debutAge;
	double dt = tEvt - population.getTime();

	return dt;
}

std::string EventDebut::getDescription(double tNow) const
{
	Person *pPerson = getPerson(0);
	return strprintf("Debut of %s", pPerson->getName().c_str());
}

void EventDebut::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson = getPerson(0);
	writeEventLogStart(true, "debut", tNow, pPerson, 0);
}

bool EventDebut::isWillingToRoutineTest(GslRandomNumberGenerator *pRndGen)
{
  Person *pPerson = getPerson(0);
  
  double x = pRndGen->pickRandomDouble();
  if(x < pPerson->getRoutineAcceptanceThreshold())
    return true;
  
  return false;
}

void EventDebut::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	assert(getNumberOfPersons() == 1);
	GslRandomNumberGenerator *pRndGen = population.getRandomNumberGenerator();
	
	Person *pPerson = getPerson(0);
	assert(pPerson != 0);

	pPerson->setSexuallyActive(t);
	
	// Schedule routine testing if enabled & willing?
	if(m_routineTestingEnabled && isWillingToRoutineTest(pRndGen)){
	  EventRoutineTesting *pEvtTesting = new EventRoutineTesting(pPerson);
	  population.onNewEvent(pEvtTesting);
	}

	// No relationships will be scheduled if the person is already in the final AIDS stage
	if (pPerson->hiv().getInfectionStage() != Person_HIV::AIDSFinal)
		population.initializeFormationEvents(pPerson, false, false, t);
}

double EventDebut::m_debutAge = -1;
bool EventDebut::m_routineTestingEnabled = false;

void EventDebut::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	bool_t r;

	if (!(r = config.getKeyValue("debut.debutage", m_debutAge, 0, 100)) ||
      !(r = config.getKeyValue("routinetesting.enabled", m_routineTestingEnabled))
      )
		abortWithMessage(r.getErrorString());
}

void EventDebut::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("debut.debutage", m_debutAge)) ||
      !(r = config.addKey("routinetesting.enabled", m_routineTestingEnabled))
      )
		abortWithMessage(r.getErrorString());
}

ConfigFunctions debutConfigFunctions(EventDebut::processConfig, EventDebut::obtainConfig, "EventDebut");

JSONConfig debutJSONConfig(R"JSON(
        "EventDebut": { 
            "depends": null, 
            "params" : [ [ "debut.debutage", 15],
                          [ "routinetesting.enabled", "no"]],
            "info": [
                "Age at which a person becomes sexually active and can form",
                "relationships"
            ]
        })JSON");

