#include "person_syphilis.h"
#include "configdistributionhelper.h"
#include "configfunctions.h"
#include "configsettings.h"
#include "configwriter.h"
#include "jsonconfig.h"
#include "person.h"
#include "personimpl.h"
#include "debugwarning.h"
#include "logsystem.h"
#include "eventsyphilistransmission.h"
#include "eventtertiarysyphilis.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include <stdlib.h>
#include <limits>
#include <vector>
#include <cmath>

using namespace std;

Person_Syphilis::Person_Syphilis(Person *pSelf): Person_STI(pSelf), m_diseaseStage(Susceptible)
{
  assert(pSelf);
  m_treated = false;
  m_diagnosed = false;
}

Person_Syphilis::~Person_Syphilis() {}

bool Person_Syphilis::isInfected() const
{
  if (m_diseaseStage != Susceptible){
    return true;
  } else {
    return false;
  }
}

bool Person_Syphilis::isInfectious() const
{
  // Infectious during primary, secondary and early latent infection stage
  if (m_diseaseStage == Primary || m_diseaseStage == Secondary){
    return true;
  } else {
    return false;
  }
}

void Person_Syphilis::writeToSyphilisStageLog(double tNow, const string &description) const
{
  assert(m_pSelf);
  int id = (int)m_pSelf->getPersonID();
  
  LogSyphilisStage.print("%10.10f,%d,%s", tNow, id, description.c_str());
}

void Person_Syphilis::setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite)
{
  assert(m_diseaseStage == Susceptible);
  assert(iType != None);
  assert(!(pOrigin == 0 && iType != Seed));
  
  m_infectionTime = t;
  m_pInfectionOrigin = pOrigin;
  m_infectionType = iType;
  m_infectionSite = iSite;
  
  // m_diseaseStage = Exposed;
  m_diseaseStage = Primary;
  writeToSyphilisStageLog(t, "Primary stage");
  
}

// void Person_Syphilis::relapse(double t)
// {
//     double rn = s_uniformDistribution.pickNumber();
//     if (rn < s_fractionRelapse) {
//       m_diseaseStage = Secondary;
//       writeToSyphilisStageLog(t, "Secondary stage (relapse)");
//     }
// }

void Person_Syphilis::progress(double t, bool treatInd)
{
  // Not treated
  if(treatInd == 0){
    if (m_diseaseStage == Primary) {
      // Primary --> Secondary & Early latent
      m_diseaseStage = Secondary;
      writeToSyphilisStageLog(t, "Secondary stage");
      
    } else if (m_diseaseStage == Secondary) {
      // Secondary --> Latent (late)
      m_diseaseStage = Latent;
      writeToSyphilisStageLog(t, "Latent stage");
      
    } else if (m_diseaseStage == Latent) {
      // Either secondary relapse or progress to tertiary infection (taken together with latent class here)
      double rn = s_uniformDistribution.pickNumber();
      if (rn < s_fractionRelapse) {
        m_diseaseStage = Secondary;
        writeToSyphilisStageLog(t, "Secondary stage (relapse)");
      } else {
        m_diseaseStage = Tertiary;
        writeToSyphilisStageLog(t, "Tertiary stage");
      }
    }
  }
  
  // Treatment
  if(treatInd == 1){
    m_diseaseStage = Susceptible;
    m_infectionTime = -1e200;
    m_recoveryTime = t;
    m_pInfectionOrigin = 0;
    m_infectionType = None;
    m_infectionSite = Vaginal;
    
    writeToSyphilisStageLog(t, "Treatment");
    
  }
  
  m_treated = treatInd;
  
}

void Person_Syphilis::diagnose(double t)
{
  m_diagnosed = true;
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
