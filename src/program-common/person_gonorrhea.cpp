#include "person_gonorrhea.h"

#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "jsonconfig.h"
#include "person.h"

Person_Gonorrhea::Person_Gonorrhea(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{}

Person_Gonorrhea::~Person_Gonorrhea() {}

bool Person_Gonorrhea::isInfected() const
{
	if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
		return true;
	} else {
		return false;
	}
}

bool Person_Gonorrhea::isInfectious() const
{
	if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
		return true;
	} else {
		return false;
	}
}

void Person_Gonorrhea::setInfected(double t, Person *pOrigin, InfectionType iType)
{
	assert(iType != None);
	assert(!(pOrigin == 0 && iType != Seed));

	m_infectionTime = t;
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	double rn = s_uniformDistribution.pickNumber();

	if (m_pSelf->isMan()) {
		if (rn < s_fractionMenSymptomatic) {
			m_diseaseStage = Symptomatic;
		} else {
			m_diseaseStage = Asymptomatic;
		}
	} else {
		if (rn < s_fractionWomenSymptomatic) {
			m_diseaseStage = Symptomatic;
		} else {
			m_diseaseStage = Asymptomatic;
		}
	}
}

void Person_Gonorrhea::progress(double t)
{
	if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic)
	{
		// Recover
		m_diseaseStage = Susceptible;

		// Reset disease variables
		m_infectionTime = -1e200;
		m_pInfectionOrigin = 0;
		m_infectionType = None;
	}
}

void Person_Gonorrhea::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	s_uniformDistribution = UniformDistribution(0, 1, pRndGen);

	bool_t r;

	if (!(r = config.getKeyValue("person.gonorrhea.fractionmensymptomatic", s_fractionMenSymptomatic)) ||
			!(r = config.getKeyValue("person.gonorrhea.fractionwomensymptomatic", s_fractionWomenSymptomatic))
		)
		abortWithMessage(r.getErrorString());
}

void Person_Gonorrhea::obtainConfig(ConfigWriter &config)
{
	bool_t r;

	if (!(r = config.addKey("person.gonorrhea.fractionmensymptomatic", s_fractionMenSymptomatic))||
			!(r = config.addKey("person.gonorrhea.fractionwomensymptomatic", s_fractionWomenSymptomatic))
		)
		abortWithMessage(r.getErrorString());
}

double Person_Gonorrhea::s_fractionMenSymptomatic = 0;
double Person_Gonorrhea::s_fractionWomenSymptomatic = 0;

UniformDistribution Person_Gonorrhea::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personGonorrheaConfigFunctions(Person_Gonorrhea::processConfig, Person_Gonorrhea::obtainConfig, "Person_Gonorrhea");

JSONConfig personGonorrheaJSONConfig(R"JSON(

        "PersonGonorrhea": {
            "depends": null,
            "params": [
				[ "person.gonorrhea.fractionmensymptomatic", 0.8 ],
				[ "person.gonorrhea.fractionwomensymptomatic", 0.5 ]
            ],
            "info": [
				"TODO"
            ]
        })JSON");

