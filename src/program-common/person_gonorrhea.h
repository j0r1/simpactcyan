#ifndef PERSON_GONORRHEA_H
#define PERSON_GONORRHEA_H

#include "person_sti.h"

#include "uniformdistribution.h"

class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;

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
	bool isInfectious() const;
	GonorrheaDiseaseStage getDiseaseStage() const { return m_diseaseStage; }

	void setInfected(double t, Person *pOrigin, InfectionType iType);
	void progress(double t);
	void setRecovered(double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	GonorrheaDiseaseStage m_diseaseStage;

	static double s_fractionMenSymptomatic;
	static double s_fractionWomenSymptomatic;

	static UniformDistribution s_uniformDistribution;
};

#endif // PERSON_GONORRHEA_H
