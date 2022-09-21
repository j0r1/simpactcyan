#include "person_chlamydia.h"

#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "person.h"

Person_Chlamydia::Person_Chlamydia(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{}

Person_Chlamydia::~Person_Chlamydia() {}

bool Person_Chlamydia::isInfected() const
{
	if (m_diseaseStage == Exposed || m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
		return true;
	} else {
		return false;
	}
}

bool Person_Chlamydia::isInfectious() const {
	if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
		return true;
	} else {
		return false;
	}
}

void Person_Chlamydia::setInfected(double t, Person *pOrigin, InfectionType iType)
{
	assert(iType != None);
	assert(!(pOrigin == 00 && iType != Seed));

	m_infectionTime = t;
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	m_diseaseStage = Exposed;
}

void Person_Chlamydia::progress(double t)
{
	if (m_diseaseStage == Exposed) {
		// Go either to symptomatic or asymptomatic stage, based on fraction symptomatic
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

	} else if (m_diseaseStage == Asymptomatic) {
		// Recover and gain temporary immunity
		m_diseaseStage = Immune;
	} else if (m_diseaseStage == Symptomatic) {
		// Recover and return to susceptible state
		m_diseaseStage = Susceptible;

		// Reset disease variables
		m_infectionTime = -1e200;
		m_pInfectionOrigin = 0;
		m_infectionType = None;

	} else if (m_diseaseStage == Immune) {
		// Return to susceptible state
		m_diseaseStage = Susceptible;

		// Reset disease variables
		m_infectionTime = -1e200;
		m_pInfectionOrigin = 0;
		m_infectionType = None;
	}
}

void Person_Chlamydia::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	s_uniformDistribution = UniformDistribution(0, 1, pRndGen);

	bool_t r;
	if (!(r = config.getKeyValue("person.chlamydia.fractionmensymptomatic", s_fractionMenSymptomatic)) ||
			!(r = config.getKeyValue("person.chlamydia.fractionwomensymptomatic", s_fractionWomenSymptomatic))
		)
		abortWithMessage(r.getErrorString());
}

void Person_Chlamydia::obtainConfig(ConfigWriter &config)
{
	bool_t r;
	if (!(r = config.addKey("person.chlamydia.fractionmensymptomatic", s_fractionMenSymptomatic)) ||
			!(r = config.addKey("person.chlamydia.fractionwomensymptomatic", s_fractionWomenSymptomatic))
		)
		abortWithMessage(r.getErrorString());
}

double Person_Chlamydia::s_fractionMenSymptomatic = 0;
double Person_Chlamydia::s_fractionWomenSymptomatic = 0;

UniformDistribution Person_Chlamydia::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personChlamydiaConfigFunctions(Person_Chlamydia::processConfig, Person_Chlamydia::obtainConfig, "Person_Chlamydia");

JSONConfig personChlamydiaJSONConfig(R"JSON(

        "PersonChlamydia": {
            "depends": null,
            "params": [
				[ "person.chlamydia.fractionmensymptomatic", 0.8 ],
				[ "person.chlamydia.fractionwomensymptomatic", 0.5 ]
            ],
            "info": [
				"TODO"
            ]
        })JSON");
