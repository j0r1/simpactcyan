#ifndef PERSON_CHLAMYDIA_H

#define PERSON_CHLAMYDIA_H

#include "person_sti.h"

enum Chlamydia_DiseaseState {
	Susceptible,
	Exposed,
	SymptomaticInfectious,
	AsymtomaticInfectious,
	Immune,
};

class Person_Chlamydia : public Person_STI
{
public:
	Person_Chlamydia();
	~Person_Chlamydia();

	bool isInfected() const;
private:
	Chlamydia_DiseaseState m_disease_state;
};

#endif // PERSON_CHLAMYDIA_H

/*
 * Feature	Chlamydia

Population

Infection stages	incubation period (not infectious; 0-28 days)
	symptomatic infection (30-40 days)
	asymptomatic infection (180-420 days)



Probability of symptomatic infection (affects health-seeking behavior)	~10%
Time from infection to symptoms	can be several weeks

Probability of acquiring HIV	increased
Probability of transmitting HIV



Treatment (initiated for symptomatic infection or after screening)	antibiotics (non-infectious after ~7 days)

Temporary immunity against reinfection	after natural clearance of asymp. infection
 */
