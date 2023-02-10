#ifndef PERSON_SYPHILIS_H
#define PERSON_SYPHILIS_H

#include "person_sti.h"
#include "util.h"
#include "uniformdistribution.h"
#include <assert.h>

class Person;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;
class ProbabilityDistribution;

class Person_Syphilis : public Person_STI
{
public:
	enum SyphilisDiseaseStage {
		Susceptible,
		// Exposed,
		Primary,
		Secondary,
		Latent,
		Tertiary
	};

	Person_Syphilis(Person *pSelf);
	virtual ~Person_Syphilis();

	bool isInfected() const;
	bool isInfectious() const;
	SyphilisDiseaseStage getDiseaseStage() const { return m_diseaseStage; }
	double getInfectionTime() const													{ assert(isInfected()); return m_infectionTime; }
	double getRecoveryTime() const                          { return m_recoveryTime; }
	bool isTreated() const                                 { return m_treated; }
	bool isDiagnosed() const                                { return m_diagnosed; }
	void setInTertiaryStage(double tNow);
	
	Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }

	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t, bool treatInd);
	void diagnose(double t);
	
	void writeToSyphilisStageLog(double tNow, const std::string &description) const;

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
  
  double m_infectionTime;
  double m_recoveryTime;
  bool m_treated;
  bool m_diagnosed;
  Person *m_pInfectionOrigin;
  
	SyphilisDiseaseStage m_diseaseStage;

	static double s_fractionRelapse; // Fraction of individuals that experience relapse into secondary syphilis during latent phases.

	static UniformDistribution s_uniformDistribution;
};

inline void Person_Syphilis::setInTertiaryStage(double tNow)
{
  m_diseaseStage = Tertiary;
  writeToSyphilisStageLog(tNow, "Tertiary stage");
}

#endif // PERSON_SYPHILIS_H