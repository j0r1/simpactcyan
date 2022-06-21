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
	Person_STI(Person *pSelf);
	virtual ~Person_STI();

	virtual bool isInfected() const = 0;

	virtual void setInfected(double t, Person *pOrigin) = 0;
	virtual void advanceDiseaseState() = 0;
	virtual void cure() = 0;

	double getInfectionTime() const 										{ assert(isInfected()); return m_infectionTime; }
	Person *getInfectionOrigin() const 										{ assert(isInfected()); return m_pInfectionOrigin; }

	// TODO declare config functions here or in child classes?

protected:
	const Person *m_pSelf;

	double m_infectionTime;
	Person *m_pInfectionOrigin;

};

#endif // PERSON_STI_H
