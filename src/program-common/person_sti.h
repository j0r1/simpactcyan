#ifndef PERSON_STI_H
#define PERSON_STI_H

#include <assert.h>

class Person;

class Person_STI {
public:
	enum InfectionType { None, Partner, Seed };

	Person_STI(Person *pSelf): m_pSelf(pSelf), m_infectionTime(-1e200), m_pInfectionOrigin(0), m_infectionType(None) {}

	virtual ~Person_STI() {}

	virtual bool isInfected() const = 0;

	InfectionType getInfectionType() const { return m_infectionType; }
	double getInfectionTime() const { assert(isInfected()); return m_infectionTime; }
	Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }

	virtual void setInfected(double t, Person *pOrigin, InfectionType iType) = 0;

protected:
	const Person *m_pSelf;

	double m_infectionTime;
	Person *m_pInfectionOrigin;
	InfectionType m_infectionType;
};

#endif // PERSON_STI_H

/*
#include "util.h"


class ProbabilityDistribution;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_HSV2
{
public:
	double getHazardAParameter() const												{ return m_hazardAParam; }
	double getHazardB2Parameter() const												{ return m_hazardB2Param; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double m_hazardAParam;
	double m_hazardB2Param;

	static ProbabilityDistribution *m_pADist;
	static ProbabilityDistribution *m_pB2Dist;
};*/
