#include "person_syphilis.h"

#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"

Person_Syphilis::Person_Syphilis(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{}

Person_Syphilis::~Person_Syphilis() {}

bool Person_Syphilis::isInfected() const
{
	if (m_diseaseStage == Exposed || m_diseaseStage == Primary || m_diseaseStage == Secondary ||
			m_diseaseStage == Latent || m_diseaseStage == Tertiary) {
		return true;
	} else {
		return false;
	}
}

bool Person_Syphilis::isInfectious() const
{
	// Infectious during primary, secondary and early latent infection stage
	return (m_diseaseStage == Primary || m_diseaseStage == Secondary);
}

void Person_Syphilis::setInfected(double t, Person *pOrigin, InfectionType iType)
{
	assert(iType != None);
	assert(!(pOrigin == 00 && iType != Seed));

	m_infectionTime = t;
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;

	m_diseaseStage = Exposed;
}

void Person_Syphilis::progress(double t)
{
	if (m_diseaseStage == Exposed) {
		// Exposed --> Primary
		m_diseaseStage = Primary;
	} else if (m_diseaseStage == Primary) {
		// Primary --> Secondary
		m_diseaseStage = Secondary;
	} else if (m_diseaseStage == Secondary) {
		// Secondary --> Latent
		m_diseaseStage = Latent;
	} else if (m_diseaseStage == Latent) {
		// Either secondary relapse or progress to tertiary infection
		double rn = s_uniformDistribution.pickNumber();
		if (rn < s_fractionRelapse) {
			m_diseaseStage = Secondary;
		} else {
			m_diseaseStage = Tertiary;
		}
	}
}

void Person_Syphilis::setRecovered(double t)
{
	// No natural clearance
}

void Person_Syphilis::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	s_uniformDistribution = UniformDistribution(0, 1, pRndGen);

	bool_t r;
	if (!(r = config.getKeyValue("person.syphilis.fractionrelapse", s_fractionRelapse)))
		abortWithMessage(r.getErrorString());
}

void Person_Syphilis::obtainConfig(ConfigWriter &config)
{
	bool_t r;
	if (!(r = config.addKey("person.syphilis.fractionrelapse", s_fractionRelapse)))
		abortWithMessage(r.getErrorString());
}

double Person_Syphilis::s_fractionRelapse = 0;

UniformDistribution Person_Syphilis::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personSyphilisConfigFunctions(Person_Syphilis::processConfig, Person_Syphilis::obtainConfig, "Person_Syphilis");

JSONConfig personSyphilisJSONConfig(R"JSON(

        "PersonSyphilis": {
            "depends": null,
            "params": [
				[ "person.syphilis.fractionrelapse", 0.25 ]
            ],
            "info": [
				"TODO"
            ]
        })JSON");
