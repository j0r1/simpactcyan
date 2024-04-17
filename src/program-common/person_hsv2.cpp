#include "person_hsv2.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "jsonconfig.h"
#include "person.h"
// #include "personimpl.h"
#include "debugwarning.h"
#include "logsystem.h"
// #include "eventhsv2transmission.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include <stdlib.h>
#include <limits>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

Person_HSV2::Person_HSV2(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{
	assert(pSelf);

	m_hazardAParam = m_pADist->pickNumber();
	m_hazardB2Param = m_pB2Dist->pickNumber();
	
	m_diagnosed = false;
}

Person_HSV2::~Person_HSV2() {}

void Person_HSV2::diagnose(double t)
{
  m_diagnosed = true;
}

bool Person_HSV2::isInfected() const
{
  // if (m_diseaseStage == Exposed || m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

bool Person_HSV2::isInfectious() const {
  // if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

void Person_HSV2::writeToHSV2StageLog(double tNow, const string &description) const
{
  assert(m_pSelf);
  int id = (int)m_pSelf->getPersonID();
  
  LogHSV2Stage.print("%10.10f,%d,%s", tNow, id, description.c_str());
}

void Person_HSV2::setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite)
{ 
  assert(m_diseaseStage == Susceptible);
	assert(iType != None);
	assert(!(pOrigin == 0 && iType != Seed));

	m_infectionTime = t; 
	m_pInfectionOrigin = pOrigin;
	m_infectionType = iType;
	m_infectionSite = iSite;

	m_diseaseStage = Primary;
	writeToHSV2StageLog(t, "Primary stage");
	
	// Symptomatic during primary stage?
	double rn = s_uniformDistribution.pickNumber();
	if(rn < s_fractionSymptomatic) {
	  m_Symptoms = Symptoms;
	}else{
	  m_Symptoms = NoSymptoms;
	}
}

void Person_HSV2::progress(double t, bool treatInd)
{
  if(m_diseaseStage == Primary)
  {
    // Progress to asymptomatic stage
    m_diseaseStage = Asymptomatic;
    writeToHSV2StageLog(t, "Asymptomatic stage");
    
  }
}

void Person_HSV2::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
	assert(pRndGen != 0);
  s_uniformDistribution = UniformDistribution(0, 1, pRndGen);
  
	delete m_pADist;
	delete m_pB2Dist;
	m_pADist = getDistributionFromConfig(config, pRndGen, "person.hsv2.a");
	m_pB2Dist = getDistributionFromConfig(config, pRndGen, "person.hsv2.b2");
	
	bool_t r;
	if(!(r = config.getKeyValue("person.hsv2.fraction.symptomatic.primary", s_fractionSymptomatic)))
	  abortWithMessage(r.getErrorString());
}

void Person_HSV2::obtainConfig(ConfigWriter &config)
{
	addDistributionToConfig(m_pADist, config, "person.hsv2.a");
	addDistributionToConfig(m_pB2Dist, config, "person.hsv2.b2");
	
	bool_t r;
	if(!(r = config.addKey("person.hsv2.fraction.symptomatic.primary", s_fractionSymptomatic)))
	  abortWithMessage(r.getErrorString());
}

ProbabilityDistribution *Person_HSV2::m_pADist = 0;
ProbabilityDistribution *Person_HSV2::m_pB2Dist = 0;
double Person_HSV2::s_fractionSymptomatic = 0;

UniformDistribution Person_HSV2::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personHSVConfigFunctions(Person_HSV2::processConfig, Person_HSV2::obtainConfig, "Person_HSV2");

JSONConfig personHSV2JSONConfig(R"JSON(

        "PersonHSV2": {
            "depends": null,
            "params": [ 
				[ "person.hsv2.a.dist", "distTypes", [ "fixed", [ [ "value", 0 ]   ] ] ],
				[ "person.hsv2.b2.dist", "distTypes", [ "fixed", [ [ "value", 0 ]   ] ] ],
        [ "person.hsv2.fraction.symptomatic.primary", 0 ]
            ],
            "info": [
				"The 'a' parameter in the HSV2 transmission hazard is chosen from this",
				"distribution, allowing transmission to depend more on the individual",
				"The 'b2' parameter in the HSV2 transmission hazard is chosen from this",
				"distribution, allowing transmission to",
				"depend more on susceptibility for HSV2 only."
            ]
        })JSON");
