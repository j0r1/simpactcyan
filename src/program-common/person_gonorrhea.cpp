#include "person_gonorrhea.h"


Person_Gonorrhea::Person_Gonorrhea(): m_infection_stage(GonorrheaInfectionStage::Susceptible) {}

Person_Gonorrhea::~Person_Gonorrhea() {}

bool Person_Gonorrhea::isInfected() const
{
	return m_infection_stage == GonorrheaInfectionStage::Exposed
			|| m_infection_stage == GonorrheaInfectionStage::AsymptomaticInfectious
			|| m_infection_stage == GonorrheaInfectionStage::SymptomaticInfectious
			|| m_infection_stage == GonorrheaInfectionStage::AsymptomaticUnderTreatment
			|| m_infection_stage == GonorrheaInfectionStage::SymptomaticUnderTreatment;
}

bool Person_Gonorrhea::isInfectious() const
{
	return m_infection_stage == GonorrheaInfectionStage::AsymptomaticInfectious
			|| m_infection_stage == GonorrheaInfectionStage::SymptomaticInfectious
			|| m_infection_stage == GonorrheaInfectionStage::AsymptomaticUnderTreatment
			|| m_infection_stage == GonorrheaInfectionStage::SymptomaticUnderTreatment;
}

bool Person_Gonorrhea::isSymptomatic() const
{
	return m_infection_stage == GonorrheaInfectionStage::SymptomaticInfectious
			|| m_infection_stage == GonorrheaInfectionStage::SymptomaticUnderTreatment;
}

void Person_Gonorrhea::setInfected(double t, Person *pOrigin)
{
	m_infection_stage = GonorrheaInfectionStage::Exposed;
	// TODO schedule disease progression event
	// TODO schedule transmission events?
	// TODO schedule treatment events?

	setInfectionTime(t);
	setInfectionOrigin(pOrigin);

}
