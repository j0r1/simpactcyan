#ifndef PERSON_SYPHILIS_H
#define PERSON_SYPHILIS_H

#include "person_sti.h"

class Person_Syphilis : public Person_STI
{
public:
	enum SyphilisDiseaseStage {
		Susceptible,
		Primary,
		Secondary,
		EarlyLatent,
		LateLatent,
		Tertiary
	};

	Person_Syphilis(Person *pSelf);
	virtual ~Person_Syphilis();

	bool isInfected() const;
	bool isInfectious() const;

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	void progress(double t);
	void setRecovered(double t);
private:
	SyphilisDiseaseStage m_diseaseStage;
};


#endif // PERSON_SYPHILIS_H
