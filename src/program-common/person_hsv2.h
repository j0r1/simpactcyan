#ifndef PERSON_HSV2_H
#define PERSON_HSV2_H

#include "person_sti.h"
#include "util.h"
// #include "uniformdistribution.h"
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

	bool isInfected() const;
	bool isInfectious() const;
	HSV2DiseaseStage getDiseaseStage() const 										{ return m_diseaseStage; }
	double getInfectionTime() const													{ assert(isInfected()); return m_infectionTime; }

	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t, bool treatInd);
	Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }

	double getHazardAParameter() const												{ return m_hazardAParam; }
	double getHazardB2Parameter() const												{ return m_hazardB2Param; }

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
  
  double m_infectionTime;
  Person *m_pInfectionOrigin;
  
	HSV2DiseaseStage m_diseaseStage;

	double m_hazardAParam;
	double m_hazardB2Param;

	static ProbabilityDistribution *m_pADist;
	static ProbabilityDistribution *m_pB2Dist;
	
	// static UniformDistribution s_uniformDistribution;
};

#endif // PERSON_HSV2_H