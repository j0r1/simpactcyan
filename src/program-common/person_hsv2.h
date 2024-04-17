#ifndef PERSON_HSV2_H
#define PERSON_HSV2_H

#include "person_sti.h"
#include "util.h"
#include "uniformdistribution.h"
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
		// Infected
		Primary,
		Asymptomatic,
		Recurrent
	};
  
  enum HSV2SymptomStatus{
    Symptoms,
    NoSymptoms
  };
  
  Person_HSV2(Person *pSelf);
	~Person_HSV2();

	bool isInfected() const;
	bool isInfectious() const;
	HSV2DiseaseStage getDiseaseStage() const 										{ return m_diseaseStage; }
	HSV2SymptomStatus getSymptoms() const                       { return m_Symptoms; }
	double getInfectionTime() const													    { assert(isInfected()); return m_infectionTime; }
	bool isDiagnosed() const                                    { return m_diagnosed; }

	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t, bool treatInd);
	void diagnose(double t);
	Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }
	
	void setInAsymptomaticStage(double tNow);
	void setInRecurrentStage(double tNow);

	double getHazardAParameter() const												{ return m_hazardAParam; }
	double getHazardB2Parameter() const												{ return m_hazardB2Param; }
	
	void writeToHSV2StageLog(double tNow, const std::string &description) const;

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
  
  double m_infectionTime;
  bool m_diagnosed;
  
  Person *m_pInfectionOrigin;
  
	HSV2DiseaseStage m_diseaseStage;
	HSV2SymptomStatus m_Symptoms;

	double m_hazardAParam;
	double m_hazardB2Param;

	static ProbabilityDistribution *m_pADist;
	static ProbabilityDistribution *m_pB2Dist;
	
	static double s_fractionSymptomatic;
	
	static UniformDistribution s_uniformDistribution;
};

inline void Person_HSV2::setInAsymptomaticStage(double tNow)
{
  m_diseaseStage = Asymptomatic;
  writeToHSV2StageLog(tNow, "Asymptomatic stage");
}

inline void Person_HSV2::setInRecurrentStage(double tNow)
{
  m_diseaseStage = Recurrent;
  writeToHSV2StageLog(tNow, "Recurrent stage");
}

#endif // PERSON_HSV2_H