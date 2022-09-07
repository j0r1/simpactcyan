#ifndef PERSON_GONORRHEA_H
#define PERSON_GONORRHEA_H

#include "person_sti.h"

class Person;

class Person_Gonorrhea : public Person_STI
{
public:
	enum GonorrheaDiseaseStage {
		Susceptible,
		Asymptomatic,
		Symptomatic
	};

	Person_Gonorrhea(Person *pSelf);
	virtual ~Person_Gonorrhea();

	bool isInfected() const;

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	void setRecovered(double t);

private:
	GonorrheaDiseaseStage m_diseaseStage;
};

#endif // PERSON_GONORRHEA_H
/*

#include "util.h"
#include <assert.h>

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
};
 */
