#ifndef PERSON_GONORRHEA_H

#define PERSON_GONORRHEA_H

#include "person_sti.h"

class ProbabilityDistribution;

enum GonorrheaInfectionStage
{
	Susceptible,
	Exposed,
	AsymptomaticInfectious,
	SymptomaticInfectious,
	AsymptomaticUnderTreatment,
	SymptomaticUnderTreatment
};

/**
 *
 */
class Person_Gonorrhea: public Person_STI
{
public:
	Person_Gonorrhea();
	virtual ~Person_Gonorrhea();

	virtual bool isInfected() const;
	virtual bool isInfectious() const;
	virtual bool isSymptomatic() const;

	virtual void setInfected(double t, Person *pOrigin);

	//virtual void advanceDiseaseState() = 0;
	//virtual void cure() = 0;

private:
	GonorrheaInfectionStage m_infection_stage;

	//static ProbabilityDistribution *m_pIncubationPeriodDist;
	// TODO infectious period length distribution
	// TODO probability of becoming symptomatic
	// TODO time until recovery distribution
	// TODO treatment
};

#endif // PERSON_GONORRHEA_H
