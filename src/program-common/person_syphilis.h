#ifndef PERSON_SYPHILIS_H
#define PERSON_SYPHILIS_H

#include "person_sti.h"

#include "uniformdistribution.h"

class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_Syphilis : public Person_STI
{
public:
	enum SyphilisDiseaseStage {
		Susceptible,
		Exposed,
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

	void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
	void progress(double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	SyphilisDiseaseStage m_diseaseStage;

	static double s_fractionRelapse; // Fraction of individuals that experience relapse into secondary syphilis during latent phases.

	static UniformDistribution s_uniformDistribution;
};


#endif // PERSON_SYPHILIS_H
