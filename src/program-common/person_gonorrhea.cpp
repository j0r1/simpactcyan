#include "person_gonorrhea.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "jsonconfig.h"
#include "person.h"
#include "personimpl.h"
#include "debugwarning.h"
#include "logsystem.h"
#include "eventgonorrheatransmission.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include <stdlib.h>
#include <limits>
#include <vector>
#include <cmath>

Person_Gonorrhea::Person_Gonorrhea(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{
  assert(pSelf);
  // 
  // assert(m_pTreatAcceptDistribution);
  // m_treatAcceptanceThreshold = m_pTreatAcceptDistribution->pickNumber();

  m_treated = false;
  m_diagnosed = false;
  // m_infectionTime = -1e200;
  // m_pInfectionOrigin = 0;
  // m_infectionType = None;
  // // m_diseaseStage = Susceptible;
  // m_infectionSite = Vaginal;
  
}

Person_Gonorrhea::~Person_Gonorrhea() {}


void Person_Gonorrhea::diagnose(double t)
{
  m_diagnosed = true;
  // m_timeLastDiagnosis = t;
  // m_diagTime = t;
}

bool Person_Gonorrhea::isInfected() const
{
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

bool Person_Gonorrhea::isInfectious() const
{
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

void Person_Gonorrhea::setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite)
{
  assert(m_diseaseStage == Susceptible);
  assert(iType != None);
  assert(!(pOrigin == 0 && iType != Seed));
  
  m_infectionTime = t;
  m_pInfectionOrigin = pOrigin;
  m_infectionType = iType;
  m_infectionSite = iSite;
  
  m_diseaseStage = Exposed;
  
  // double rn = s_uniformDistribution.pickNumber();
  // 
  // if (m_pSelf->isMan() && iSite == 1) {
  //   if (rn < s_fractionMenSymptomaticRectal) {
  //     m_diseaseStage = Symptomatic;
  //   } else {
  //     m_diseaseStage = Asymptomatic;
  //   }
  // }
  // else if (m_pSelf->isMan() && iSite == 2) {
  //   if (rn < s_fractionMenSymptomaticUrethral) {
  //     m_diseaseStage = Symptomatic;
  //   } else {
  //     m_diseaseStage = Asymptomatic;
  //   }
  // }
  // else {
  //   if (rn < s_fractionWomenSymptomatic) {
  //     m_diseaseStage = Symptomatic;
  //   } else {
  //     m_diseaseStage = Asymptomatic;
  //   }
  // }
  
  
}

void Person_Gonorrhea::progress(double t, bool treatInd) // recovery (= progression)
{
  if (m_diseaseStage == Exposed) {
    // Go either to symptomatic or asymptomatic stage, based on fraction symptomatic
    double rn = s_uniformDistribution.pickNumber();
    
    if (m_pSelf->isMan() && m_infectionSite == Rectal) {
      if (rn < s_fractionMenSymptomaticRectal) {
        m_diseaseStage = Symptomatic;
      } else {
        m_diseaseStage = Asymptomatic;
      }
    }
    else if (m_pSelf->isMan() && m_infectionSite == Urethral) {
      if (rn < s_fractionMenSymptomaticUrethral) {
        m_diseaseStage = Symptomatic;
      } else {
        m_diseaseStage = Asymptomatic;
      }
    }
    else {
      if (rn < s_fractionWomenSymptomatic) {
        m_diseaseStage = Symptomatic;
      } else {
        m_diseaseStage = Asymptomatic;
      }
    }
    
  } else if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic)
  {
    // Recover
    m_diseaseStage = Susceptible;
    
    // Reset disease variables
    m_infectionTime = -1e200;
    m_recoveryTime = t;
    m_pInfectionOrigin = 0;
    m_infectionType = None;
    m_infectionSite = Vaginal;
    m_treated = treatInd;
    // m_diagTime = -1e200;
    m_diagnosed = false;
    
  }
  
  // Person *pPerson;
  // pPerson->writeToGonorrheaTreatLog();
}

double Person_Gonorrhea::s_fractionMenSymptomaticRectal = 0;
double Person_Gonorrhea::s_fractionMenSymptomaticUrethral = 0;
double Person_Gonorrhea::s_fractionWomenSymptomatic = 0;

void Person_Gonorrhea::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  s_uniformDistribution = UniformDistribution(0, 1, pRndGen);
  
  bool_t r;
  
  if (!(r = config.getKeyValue("person.gonorrhea.fractionmensymptomatic.rectal", s_fractionMenSymptomaticRectal)) ||
      !(r = config.getKeyValue("person.gonorrhea.fractionmensymptomatic.urethral", s_fractionMenSymptomaticUrethral)) ||
      !(r = config.getKeyValue("person.gonorrhea.fractionwomensymptomatic", s_fractionWomenSymptomatic))
  )
    abortWithMessage(r.getErrorString());
}

void Person_Gonorrhea::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  
  if (!(r = config.addKey("person.gonorrhea.fractionmensymptomatic.rectal", s_fractionMenSymptomaticRectal))||
      !(r = config.addKey("person.gonorrhea.fractionmensymptomatic.urethral", s_fractionMenSymptomaticUrethral))||
      !(r = config.addKey("person.gonorrhea.fractionwomensymptomatic", s_fractionWomenSymptomatic))
  )
    abortWithMessage(r.getErrorString());
  
  // addDistributionToConfig(m_pTreatAcceptDistribution, config, "person.sti.treat.accept.threshold");
  
}

UniformDistribution Person_Gonorrhea::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personGonorrheaConfigFunctions(Person_Gonorrhea::processConfig, Person_Gonorrhea::obtainConfig, "Person_Gonorrhea");

JSONConfig personGonorrheaJSONConfig(R"JSON(
    
    "PersonGonorrhea": {
  "depends": null,
  "params": [
  [ "person.gonorrhea.fractionmensymptomatic.rectal", 0.1 ],
  [ "person.gonorrhea.fractionmensymptomatic.urethral", 0.8 ],
  [ "person.gonorrhea.fractionwomensymptomatic", 0.5 ]
  ],
            "info": [
  "These parameters specify the fraction of symptomatic infections for men and women"
            ]
    })JSON");
