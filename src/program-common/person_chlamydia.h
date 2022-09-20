#ifndef PERSON_CHLAMYDIA_H
#define PERSON_CHLAMYDIA_H

#include "person_sti.h"

class Person_Chlamydia : public Person_STI
{
public:
	enum ChlamydiaDiseaseStage {
		Susceptible,
		Asymptomatic,
		Symptomatic,
		Immune
	};

	Person_Chlamydia(Person *pSelf);
	virtual ~Person_Chlamydia();

	bool isInfected() const;

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	void setRecovered(double t);

private:
	ChlamydiaDiseaseStage m_diseaseStage;
};

#endif // PERSON_CHLAMYDIA_H
