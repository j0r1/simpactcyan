#ifndef PERSON_HSV2_H
#define PERSON_HSV2_H

#include "person_sti.h"

#include "util.h"
#include <assert.h>

class Person;
class ProbabilityDistribution;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_HSV2 : public Person_STI
{
public:
	enum HSV2DiseaseStage {
		Susceptible,
		Infected
	};

	Person_HSV2(Person *pSelf);
	~Person_HSV2();

	bool isInfected() const															{ return m_diseaseStage == Infected; }
	bool isInfectious() const														{ return m_diseaseStage == Infected;}
	HSV2DiseaseStage getDiseaseStage() const 										{ return m_diseaseStage; }

	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t);

	double getHazardAParameter() const												{ return m_hazardAParam; }
	double getHazardB2Parameter() const												{ return m_hazardB2Param; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	HSV2DiseaseStage m_diseaseStage;

	double m_hazardAParam;
	double m_hazardB2Param;

	static ProbabilityDistribution *m_pADist;
	static ProbabilityDistribution *m_pB2Dist;
};

#endif // PERSON_HSV2_H
