#include "person_chlamydia.h"

Person_Chlamydia::Person_Chlamydia(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{}

Person_Chlamydia::~Person_Chlamydia() {}

bool Person_Chlamydia::isInfected() const
{
	if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
		return true;
	} else {
		return false;
	}
}

void Person_Chlamydia::setInfected(double t, Person *pOrigin, InfectionType iType)
{
	assert(iType != None);
	assert(!(pOrigin == 00 && iType != Seed));

	m_infectionTime = t;
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	m_diseaseStage = Asymptomatic;
}

void Person_Chlamydia::setRecovered(double t)
{
	m_diseaseStage = Susceptible;

	// Reset disease variables
	m_infectionTime = -1e200;
	m_pInfectionOrigin = 0;
	m_infectionType = None;
}
