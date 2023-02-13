#ifndef PERSON_CHLAMYDIA_H
#define PERSON_CHLAMYDIA_H

#include "person_sti.h"
#include "util.h"
#include "uniformdistribution.h"
#include <assert.h>

class Person;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;
class ProbabilityDistribution;

class Person_Chlamydia : public Person_STI
{
public:
	enum ChlamydiaDiseaseStage {
		Susceptible,
		// Exposed,
		Asymptomatic,
		Symptomatic
		// Immune
	};

	Person_Chlamydia(Person *pSelf);
	virtual ~Person_Chlamydia();

	bool isInfected() const;
	bool isInfectious() const;
	ChlamydiaDiseaseStage getDiseaseStage() const { return m_diseaseStage; }
	// bool isImmune() const { return m_diseaseStage == Immune; }
	double getInfectionTime() const													{ assert(isInfected()); return m_infectionTime; }
	double getRecoveryTime() const                          { return m_recoveryTime; }
	bool isTreated() const                                 { return m_treated; }
	bool isDiagnosed() const                                { return m_diagnosed; }

	Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }
	
	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t, bool treatInd);
	void diagnose(double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
  
  double m_infectionTime;
  double m_recoveryTime;
  bool m_treated;
  bool m_diagnosed;
  Person *m_pInfectionOrigin;
  
	ChlamydiaDiseaseStage m_diseaseStage;

	static double s_fractionMenSymptomaticRectal;
	static double s_fractionMenSymptomaticUrethral;
	static double s_fractionWomenSymptomatic;

	static UniformDistribution s_uniformDistribution;
};

#endif // PERSON_CHLAMYDIA_H