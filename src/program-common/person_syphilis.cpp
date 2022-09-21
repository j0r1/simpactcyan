#include "person_syphilis.h"

Person_Syphilis::Person_Syphilis(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{}

Person_Syphilis::~Person_Syphilis() {}

bool Person_Syphilis::isInfected() const
{
	if (m_diseaseStage == Primary || m_diseaseStage == Secondary || m_diseaseStage == EarlyLatent
			|| m_diseaseStage == LateLatent || m_diseaseStage == Tertiary) {
		return true;
	} else {
		return false;
	}
}

bool Person_Syphilis::isInfectious() const
{
	return isInfected();
}

void Person_Syphilis::setInfected(double t, Person *pOrigin, InfectionType iType)
{
	assert(iType != None);
	assert(!(pOrigin == 00 && iType != Seed));

	m_infectionTime = t;
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	m_diseaseStage = Primary;
}

void Person_Syphilis::progress(double t)
{

}

void Person_Syphilis::setRecovered(double t)
{
	m_diseaseStage = Susceptible;

	// Reset disease variables
	m_infectionTime = -1e200;
	m_pInfectionOrigin = 0;
	m_infectionType = None;
}
