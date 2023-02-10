#include "eventformation.h"
#include "eventdissolution.h"
#include "eventhivtransmission.h"
#include "eventhsv2transmission.h"
#include "eventgonorrheatransmission.h"
#include "eventchlamydiatransmission.h"
#include "eventsyphilistransmission.h"
#include "eventdebut.h"
#include "simpactpopulation.h"
#include "simpactevent.h"
#include "evthazardformationsimple.h"
#include "evthazardformationagegap.h"
#include "evthazardformationagegaprefyear.h"
#include "eventconception.h"
#include "eventprepoffered.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include "util.h"
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace std;

EventFormation::EventFormation(Person *pPerson1, Person *pPerson2, double lastDissTime, double formationScheduleTime) 
	: SimpactEvent(pPerson1, pPerson2),
	  m_lastDissolutionTime(lastDissTime),
	  m_formationScheduleTime(formationScheduleTime)
{
	assert(pPerson1->isMan());
	assert(pPerson1 != pPerson2); // Never form a relationship with ourselves
#ifndef NDEBUG
	if (pPerson2->isMan()) // MSM is also possible
	{
		// let's keep things ordered, to help avoid double events
		assert(pPerson1->getPersonID() < pPerson2->getPersonID()); 
	}
	else
		assert(pPerson2->isWoman());
#endif // NDEBUG

	assert(pPerson1->isSexuallyActive());
	assert(pPerson2->isSexuallyActive());

	// New formation events must not be scheduled if one of the persons is in the
	// final AIDS stage
	assert(pPerson1->hiv().getInfectionStage() != Person_HIV::AIDSFinal);
	assert(pPerson2->hiv().getInfectionStage() != Person_HIV::AIDSFinal);
}

EventFormation::~EventFormation()
{
}

string EventFormation::getDescription(double tNow) const
{
	string evtName = (getPerson(1)->isWoman()) ? "Formation" : "MSM Formation";
	return strprintf("%s between %s and %s", evtName.c_str(), getPerson(0)->getName().c_str(), getPerson(1)->getName().c_str());
}

void EventFormation::writeLogs(const SimpactPopulation &pop, double tNow) const
{
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	string evtName = (pPerson2->isWoman()) ? "formation" : "formationmsm";
	writeEventLogStart(true, evtName, tNow, pPerson1, pPerson2);
}

bool EventFormation::isUseless(const PopulationStateInterface &pop)
{
	// Formation event becomes useless if one of the people is in the final AIDS
	// stage
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);
	
	if (pPerson1->hiv().getInfectionStage() == Person_HIV::AIDSFinal || pPerson2->hiv().getInfectionStage() == Person_HIV::AIDSFinal)
		return true;
	
	// Formation event becomes useless if sexual roles are not compatible (only for MSM)
	if (pPerson1->isMan() && pPerson2->isMan())
	{
	  int p1Role = pPerson1->getPreferredSexualRole();
	  int p2Role = pPerson2->getPreferredSexualRole();
	  
	  // not compatible if both the same role and not versatile
	  if (p1Role == p2Role && p1Role != 0)
	  {
	    return true;
	  }
	}

	// Check if old formation events need to be dropped because a person moved

	const SimpactPopulation &population = SIMPACTPOPULATION(&pop);
	const double eyeCapFrac = population.getEyeCapsFraction();

	if (eyeCapFrac >= 1.0)
	{
		// Nothing to do, no relationships will be changed because everyone
		// will have everyone else as a potential partner
	}
	else
	{
		const double locTime1 = pPerson1->getLocationTime();
		const double locTime2 = pPerson2->getLocationTime();

		if (locTime1 > m_formationScheduleTime || locTime2 > m_formationScheduleTime)
		{
			// TODO: remove this
			//cerr << "Formation event between " << pPerson1->getName() << " and " << pPerson2->getName() << 
			//	    " became irrelevant because of a move" << endl;

			return true;
		}
	}

	return false;
}

void EventFormation::fire(Algorithm *pAlgorithm, State *pState, double t)
{
	SimpactPopulation &population = SIMPACTPOPULATION(pState);
	Person *pPerson1 = getPerson(0);
	Person *pPerson2 = getPerson(1);

	pPerson1->addRelationship(pPerson2, t);
	pPerson2->addRelationship(pPerson1, t);

	// Need to add a dissolution event

	EventDissolution *pDissEvent = new EventDissolution(pPerson1, pPerson2, t);
	population.onNewEvent(pDissEvent);

	// In case it's a man/woman relationship, conception is possible

	if (pPerson2->isWoman())
	{
		Woman *pWoman = WOMAN(pPerson2);

		if (!pWoman->isPregnant())
		{
			EventConception *pEvtConception = new EventConception(pPerson1, pPerson2, t); // should be: man, woman, formation time
			population.onNewEvent(pEvtConception);
		}
	}

	// If one of the partners is infected (but not both), schedule a
	// transmission event

	if (pPerson1->hiv().isInfected())
	{
		if (!pPerson2->hiv().isInfected())
		{
			EventHIVTransmission *pEvtTrans = new EventHIVTransmission(pPerson1, pPerson2);
			population.onNewEvent(pEvtTrans);
		}
	}
	else // pPerson1 not infected
	{
		if (pPerson2->hiv().isInfected())
		{
			EventHIVTransmission *pEvtTrans = new EventHIVTransmission(pPerson2, pPerson1);
			population.onNewEvent(pEvtTrans);
		} 
	}
	
	// Same for HSV2

	if (pPerson1->hsv2().isInfected())
	{
		if (!pPerson2->hsv2().isInfected())
		{
			EventHSV2Transmission *pEvtTrans = new EventHSV2Transmission(pPerson1, pPerson2);
			population.onNewEvent(pEvtTrans);
		}
	}
	else // pPerson1 not infected
	{
		if (pPerson2->hsv2().isInfected())
		{
			EventHSV2Transmission *pEvtTrans = new EventHSV2Transmission(pPerson2, pPerson1);
			population.onNewEvent(pEvtTrans);
		} 
	}
	
	// Gonorrhea transmission event
	if (pPerson1->gonorrhea().isInfected())
	{
	  if (!pPerson2->gonorrhea().isInfected())
	  {
	    EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pPerson1, pPerson2);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	else // pPerson1 not infected
	{
	  if (pPerson2->gonorrhea().isInfected())
	  {
	    EventGonorrheaTransmission *pEvtTrans = new EventGonorrheaTransmission(pPerson2, pPerson1);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	
	// Chlamydia transmission event
	if (pPerson1->chlamydia().isInfected())
	{
	  if (!pPerson2->chlamydia().isInfected())
	  {
	    EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pPerson1, pPerson2);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	else // pPerson1 not infected
	{
	  if (pPerson2->chlamydia().isInfected())
	  {
	    EventChlamydiaTransmission *pEvtTrans = new EventChlamydiaTransmission(pPerson2, pPerson1);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	
	// Syphilis transmission event
	if (pPerson1->syphilis().isInfected())
	{
	  if (!pPerson2->syphilis().isInfected())
	  {
	    EventSyphilisTransmission *pEvtTrans = new EventSyphilisTransmission(pPerson1, pPerson2);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	else // pPerson1 not infected
	{
	  if (pPerson2->syphilis().isInfected())
	  {
	    EventSyphilisTransmission *pEvtTrans = new EventSyphilisTransmission(pPerson2, pPerson1);
	    population.onNewEvent(pEvtTrans);
	  }
	}
	

	// Update PreP eligibility for both persons
	bool schedulePrePOfferedEventP1 = pPerson1->hiv().updatePrePEligibility(t);
	bool schedulePrePOfferedEventP2 = pPerson2->hiv().updatePrePEligibility(t);

	// Check if either person has become eligible for PreP
	if (schedulePrePOfferedEventP1) {
		// Schedule PreP being offered to this person
		EventPrePOffered *pEvtPreP = new EventPrePOffered(pPerson1);
		population.onNewEvent(pEvtPreP);
	}

	if (schedulePrePOfferedEventP2) {
		// Schedule PreP being offered to this person
		EventPrePOffered *pEvtPreP = new EventPrePOffered(pPerson2);
		population.onNewEvent(pEvtPreP);
	}

}

double EventFormation::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	Person *pPerson2 = getPerson(1);
	EvtHazard *pHazard = (pPerson2->isWoman()) ? m_pHazard : m_pHazardMSM; 
	assert(pHazard != 0);

	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	return pHazard->calculateInternalTimeInterval(population, *this, t0, dt);
}

double EventFormation::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	Person *pPerson2 = getPerson(1);
	EvtHazard *pHazard = (pPerson2->isWoman()) ? m_pHazard : m_pHazardMSM; 
	assert(pHazard != 0);

	const SimpactPopulation &population = SIMPACTPOPULATION(pState);
	return pHazard->solveForRealTimeInterval(population, *this, Tdiff, t0);
}

EvtHazard *EventFormation::m_pHazard = 0;
EvtHazard *EventFormation::m_pHazardMSM = 0;

EvtHazard *EventFormation::getHazard(ConfigSettings &config, const string &prefix, bool msm)
{
	string hazardType;
	bool_t r;

	string hazSimple = "simple";
	string hazAgegap = "agegap";
	string hazRefyear = "agegapry";

	vector<string> allowedValues = { hazSimple, hazAgegap, hazRefyear };

	if (!(r = config.getKeyValue(prefix + ".type", hazardType, allowedValues)))
		abortWithMessage(r.getErrorString());

	// Here, the MSM flag is used to set Dp to zero in the hazard
	if (hazardType == hazSimple)
		return EvtHazardFormationSimple::processConfig(config, prefix, hazSimple, msm);
	
	if (hazardType == hazAgegap)
		return EvtHazardFormationAgeGap::processConfig(config, prefix, hazAgegap, msm);
	
	if (hazardType == hazRefyear)
		return EvtHazardFormationAgeGapRefYear::processConfig(config, prefix, hazRefyear, msm);
	
	// In case it's not covered by 'allowedValues'
	abortWithMessage("Unknown " + prefix + ".type value: " + hazardType);
	return 0; // Won't get here but otherwise there's a compiler warning
}

void EventFormation::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	delete m_pHazard;
	m_pHazard = getHazard(config, "formation.hazard", false);

	delete m_pHazardMSM;
	m_pHazardMSM = getHazard(config, "formationmsm.hazard", true);
}

void EventFormation::obtainConfig(ConfigWriter &config)
{
	if (!m_pHazard)
		abortWithMessage("EventFormation::obtainConfig: m_pHazard is null");
	if (!m_pHazardMSM)
		abortWithMessage("EventFormation::obtainConfig: m_pHazardMSM is null");

	m_pHazard->obtainConfig(config, "formation.hazard");
	m_pHazardMSM->obtainConfig(config, "formationmsm.hazard");
}

ConfigFunctions formationConfigFunctions(EventFormation::processConfig, EventFormation::obtainConfig, "EventFormation");

JSONConfig formationTypesJSONConfig(R"JSON(
        "EventFormationTypes": { 
            "depends": null,
            "params": [ ["formation.hazard.type", "agegap", [ "simple", "agegap", "agegapry" ] ] ],
            "info": null 
        })JSON");

JSONConfig formationMSMTypesJSONConfig(R"JSON(
        "EventFormationMSMTypes": { 
            "depends": null,
            "params": [ ["formationmsm.hazard.type", "agegap", [ "simple", "agegap", "agegapry" ] ] ],
            "info": null 
        })JSON");


