#ifndef PERSON_CHLAMYDIA_H
#define PERSON_CHLAMYDIA_H

#include "person_sti.h"

#include "uniformdistribution.h"

class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

class Person_Chlamydia : public Person_STI
{
public:
	enum ChlamydiaDiseaseStage {
		Susceptible,
		Exposed,
		Asymptomatic,
		Symptomatic,
		Immune
	};

	Person_Chlamydia(Person *pSelf);
	virtual ~Person_Chlamydia();

	bool isInfected() const;
	bool isInfectious() const;
	ChlamydiaDiseaseStage getDiseaseStage() const { return m_diseaseStage; }
	bool isImmune() const { return m_diseaseStage == Immune; }

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	void progress(double t);
	void setRecovered(double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	ChlamydiaDiseaseStage m_diseaseStage;

	static double s_fractionMenSymptomatic;
	static double s_fractionWomenSymptomatic;

	static UniformDistribution s_uniformDistribution;
};

#endif // PERSON_CHLAMYDIA_H
