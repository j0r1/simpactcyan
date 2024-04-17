#include "person_chlamydia.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "jsonconfig.h"
#include "person.h"
#include "personimpl.h"
#include "debugwarning.h"
#include "logsystem.h"
#include "eventchlamydiatransmission.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include <stdlib.h>
#include <limits>
#include <vector>
#include <cmath>

Person_Chlamydia::Person_Chlamydia(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{
  assert(pSelf);
  m_treated = false;
  m_diagnosed = false;
}

Person_Chlamydia::~Person_Chlamydia() {}

bool Person_Chlamydia::isInfected() const
{
  // if (m_diseaseStage == Exposed || m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

bool Person_Chlamydia::isInfectious() const {
  // if (m_diseaseStage == Asymptomatic || m_diseaseStage == Symptomatic) {
  if (m_diseaseStage != Susceptible) {
    return true;
  } else {
    return false;
  }
}

void Person_Chlamydia::setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite)
{
  assert(m_diseaseStage == Susceptible);
  assert(iType != None);
  assert(!(pOrigin == 0 && iType != Seed));
  
  m_infectionTime = t;
  m_pInfectionOrigin = pOrigin;
  m_infectionType = iType;
  m_infectionSite = iSite;
  
  // double rn = s_uniformDistribution.pickNumber();
  
  m_diseaseStage = Exposed;
  
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

void Person_Chlamydia::progress(double t, bool treatInd)
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
    m_diagnosed = false;
    
  }

}

void Person_Chlamydia::diagnose(double t)
{
  m_diagnosed = true;
}

double Person_Chlamydia::s_fractionMenSymptomaticRectal = 0;
double Person_Chlamydia::s_fractionMenSymptomaticUrethral = 0;
double Person_Chlamydia::s_fractionWomenSymptomatic = 0;


void Person_Chlamydia::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  s_uniformDistribution = UniformDistribution(0, 1, pRndGen);
  
  bool_t r;
  if (!(r = config.getKeyValue("person.chlamydia.fractionmensymptomatic.rectal", s_fractionMenSymptomaticRectal)) ||
      !(r = config.getKeyValue("person.chlamydia.fractionmensymptomatic.urethral", s_fractionMenSymptomaticUrethral)) ||
      !(r = config.getKeyValue("person.chlamydia.fractionwomensymptomatic", s_fractionWomenSymptomatic))
  )
    abortWithMessage(r.getErrorString());
}

void Person_Chlamydia::obtainConfig(ConfigWriter &config)
{
  bool_t r;
  if (!(r = config.addKey("person.chlamydia.fractionmensymptomatic.rectal", s_fractionMenSymptomaticRectal))||
      !(r = config.addKey("person.chlamydia.fractionmensymptomatic.urethral", s_fractionMenSymptomaticUrethral))||
      !(r = config.addKey("person.chlamydia.fractionwomensymptomatic", s_fractionWomenSymptomatic))
  )
    abortWithMessage(r.getErrorString());
}

UniformDistribution Person_Chlamydia::s_uniformDistribution = UniformDistribution(0, 1, 0);

ConfigFunctions personChlamydiaConfigFunctions(Person_Chlamydia::processConfig, Person_Chlamydia::obtainConfig, "Person_Chlamydia");

JSONConfig personChlamydiaJSONConfig(R"JSON(
    
    "PersonChlamydia": {
  "depends": null,
  "params": [
  [ "person.chlamydia.fractionmensymptomatic.rectal", 0 ],
  [ "person.chlamydia.fractionmensymptomatic.urethral", 0.1 ],
  [ "person.chlamydia.fractionwomensymptomatic", 0.3 ]
  ],
            "info": [
  "TODO"
            ]
})JSON");
