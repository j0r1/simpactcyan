#ifndef PERSON_STI_H

#define PERSON_STI_H

#include <assert.h>

class Person;

/**
 * Abstract base class for STI co-infections
 */
class Person_STI
{
public:
	Person_STI(): m_infectionTime(-1e200), m_pInfectionOrigin(0) {}
	virtual ~Person_STI() {}

	virtual bool isInfected() const = 0;
	virtual bool isInfectious() const = 0;
	virtual bool isSymptomatic() const = 0;

	virtual void setInfected(double t, Person *pOrigin) = 0;
	//virtual void advanceDiseaseState() = 0;
	//virtual void cure() = 0;

	double getInfectionTime() const 										{ assert(isInfected()); return m_infectionTime; }
	Person *getInfectionOrigin() const 										{ assert(isInfected()); return m_pInfectionOrigin; }

protected:
	void setInfectionTime(double t) 										{ m_infectionTime = t; }
	void setInfectionOrigin(Person *pOrigin) 								{ m_pInfectionOrigin = pOrigin; }

	double m_infectionTime;
	Person *m_pInfectionOrigin;

};

#endif // PERSON_STI_H
