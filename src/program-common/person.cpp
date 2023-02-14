#include "person.h"
#include "personimpl.h"
#include "configsettings.h"
#include "configwriter.h"
#include "configdistributionhelper.h"
#include "debugwarning.h"
#include "logsystem.h"
#include "simpactevent.h"
#include "discretedistribution2d.h"
#include "jsonconfig.h"
#include "configfunctions.h"
#include <stdlib.h>
#include <limits>

using namespace std;

Person::Person(double dateOfBirth, Gender g) : PersonBase(g, dateOfBirth), m_relations(this), m_hiv(this),
m_chlamydia(this), m_hsv2(this), m_gonorrhea(this), m_syphilis(this), m_condom_use_threshold(0),
m_health_seeking_propensity(0), m_sexual_role_preference(SexualRolePreference::Variable), m_treatAcceptanceThreshold(0)
{
  assert(g == Male || g == Female);
  
  assert(m_pPopDist);
  assert(m_pHealthSeekingPropensityDist);
  assert(m_pCondomUseDist);
  
  Point2D loc = m_pPopDist->pickPoint();
  assert(loc.x == loc.x && loc.y == loc.y); // check for NaN
  setLocation(loc, 0);
  
  double hsp = m_pHealthSeekingPropensityDist->pickNumber();
  assert(hsp == hsp); // check for NaN
  setHealthSeekingPropensity(hsp);
  
  assert(m_pTreatAcceptDist);
  double trtacc = m_pTreatAcceptDist->pickNumber();
  assert(trtacc == trtacc); // check for NaN
  m_treatAcceptanceThreshold = trtacc;
  
  double cup = m_pCondomUseDist->pickNumber();
  assert(cup == cup); // check for NaN
  m_condom_use_threshold = cup;
  
  if(isMan()){
    int sexrole = round(m_pSexualRoleDist->pickNumber());
    m_sexual_role_preference = static_cast<SexualRolePreference>(sexrole);	  
  }// for Women this is 0 (Variable) by default then?
  
  m_pPersonImpl = new PersonImpl(*this);
  
  m_STIdiagnoseCount = 0;
  m_timeLastDiagnosis = -10;
  
}

Person::~Person()
{
  delete m_pPersonImpl;
}

ProbabilityDistribution2D *Person::m_pPopDist = 0;
double Person::m_popDistWidth = 0;
double Person::m_popDistHeight = 0;

ProbabilityDistribution *Person::m_pHealthSeekingPropensityDist = 0;
ProbabilityDistribution *Person::m_pTreatAcceptDist = 0;


ProbabilityDistribution *Person::m_pCondomUseDist = 0;
double Person::m_concordanceCondomUseFactor = 1;
double Person::m_artCondomUseFactor = 1;
double Person::m_prepCondomUseFactor = 1;

ProbabilityDistribution *Person::m_pSexualRoleDist = 0;

void Person::increaseSTIDiagnoseCount(double tNow, double lastSTItime){
  
  // if last STI more than one year ago
  if(lastSTItime < (tNow - 1)){
    m_STIdiagnoseCount = 1;
    m_timeLastDiagnosis = tNow;
  }else{
    m_STIdiagnoseCount++;
    // m_STIdiagnoseCount++;
    m_timeLastDiagnosis = tNow;
  }
    
}


bool Person::usesCondom(bool isPartnerDiagnosed, GslRandomNumberGenerator *pRndGen) const
{
  double relationshipCondomUseThreshold = m_condom_use_threshold;
  // Adjust for concordance of perceived HIV serostatus
  bool amIDiagnosed = m_hiv.isDiagnosed();
  if (isPartnerDiagnosed == amIDiagnosed) {
    // Concordant HIV status
    relationshipCondomUseThreshold *= m_concordanceCondomUseFactor;
  }
  
  if (m_hiv.isInfected()) {
    // Adjust for ART use in case person is HIV positive
    if (m_hiv.hasLoweredViralLoad()) {
      relationshipCondomUseThreshold *= m_artCondomUseFactor;
    }
    
  } else {
    // Adjust for PreP use in case person is HIV negative
    if (m_hiv.isOnPreP()) {
      relationshipCondomUseThreshold *= m_prepCondomUseFactor;
    }
  }
  
  double rn = pRndGen->pickRandomDouble();
  if (rn < relationshipCondomUseThreshold)
  {
    return true;
  } else {
    return false;
  }
  
}

bool Person::isInfectedWithSTI() const
{
  return (m_chlamydia.isInfected() || m_gonorrhea.isInfected() || m_hsv2.isInfected() || m_syphilis.isInfected());
}

void Person::processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen)
{
  assert(pRndGen != 0);
  
  // Population distribution
  delete m_pPopDist;
  m_pPopDist = getDistribution2DFromConfig(config, pRndGen, "person.geo");
  
  // Health-seeking propensity distribution
  delete m_pHealthSeekingPropensityDist;
  m_pHealthSeekingPropensityDist = getDistributionFromConfig(config, pRndGen, "person.healthseekingpropensity");
  
  // Condom use probability distributions
  delete m_pCondomUseDist;
  m_pCondomUseDist = getDistributionFromConfig(config, pRndGen, "person.condomuse");
  // Condom use adjustment factors
  config.getKeyValue("person.condomuse.concordancefactor", m_concordanceCondomUseFactor);
  config.getKeyValue("person.condomuse.artfactor", m_artCondomUseFactor);
  config.getKeyValue("person.condomuse.prepfactor", m_prepCondomUseFactor);
  
  // STI treatment acceptance
  delete m_pTreatAcceptDist;
  m_pTreatAcceptDist = getDistributionFromConfig(config, pRndGen, "person.sti.treat.accept.threshold");
  
  // Sexual role preference distribution
  delete m_pSexualRoleDist;
  m_pSexualRoleDist = getDistributionFromConfig(config, pRndGen, "person.sexualrole");
}

void Person::obtainConfig(ConfigWriter &config)
{
  assert(m_pPopDist);
  addDistribution2DToConfig(m_pPopDist, config, "person.geo");
  assert(m_pHealthSeekingPropensityDist);
  addDistributionToConfig(m_pHealthSeekingPropensityDist, config, "person.healthseekingpropensity");
  assert(m_pCondomUseDist);
  addDistributionToConfig(m_pCondomUseDist, config, "person.condomuse");
  config.addKey("person.condomuse.concordancefactor", m_concordanceCondomUseFactor);
  config.addKey("person.condomuse.artfactor", m_artCondomUseFactor);
  config.addKey("person.condomuse.prepfactor", m_prepCondomUseFactor);
  addDistributionToConfig(m_pSexualRoleDist, config, "person.sexualrole");
  assert(m_pTreatAcceptDist);
  addDistributionToConfig(m_pTreatAcceptDist, config, "person.sti.treat.accept.threshold");
  
}

void Person::writeToPersonLog()
{
  double infinity = numeric_limits<double>::infinity();
  double NaN = numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  int gender = (isMan())?0:1;
  double timeOfBirth = getDateOfBirth();
  double timeOfDeath = (hasDied())?getTimeOfDeath():infinity;
  
  Man *pFather = getFather();
  Woman *pMother = getMother();
  int fatherID = (pFather != 0) ? (int)pFather->getPersonID() : (-1); // TODO: cast should be ok
  int motherID = (pMother != 0) ? (int)pMother->getPersonID() : (-1);
  
  // TODO: Currently not keeping track of children
  
  double debutTime = (isSexuallyActive())? m_relations.getDebutTime():infinity;
  double formationEagerness = getFormationEagernessParameter();
  double formationEagernessMSM = (isMan())?getFormationEagernessParameterMSM():NaN;
  int SexualRole = getPreferredSexualRole();
  
  double infectionTime = (m_hiv.isInfected())? m_hiv.getInfectionTime() : infinity;
  Person *pOrigin = (m_hiv.isInfected())? m_hiv.getInfectionOrigin() : 0;
  int origin = (pOrigin != 0) ? (int)pOrigin->getPersonID() : (-1); // TODO: cast should be ok
  
  int STIdiagnoses = m_STIdiagnoseCount;
  
  int infectionType = 0;
  switch(m_hiv.getInfectionType())
  {
  case Person_HIV::None:
    infectionType = -1;
    break;
  case Person_HIV::Seed:
    infectionType = 0;
    break;
  case Person_HIV::Partner:
    infectionType = 1;
    break;
  case Person_HIV::Mother:
    infectionType = 2;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionType = 10000 + (int)m_hiv.getInfectionType();
  }
  
  int infectionSite = 0;
  switch(m_hiv.getInfectionSite())
  {
  case Person_HIV::Vaginal:
    infectionSite = 0;
    break;
  case Person_HIV::Rectal:
    infectionSite = 1;
    break;
  case Person_HIV::Urethral:
    infectionSite = 2;
    break;
  default:
    infectionSite = 10000 + (int)m_hiv.getInfectionSite();
  }
  
  double log10SPVLoriginal = (m_hiv.isInfected()) ? std::log10(m_hiv.getOriginalViralLoad()) : -infinity;
  double treatmentTime = (m_hiv.isInfected() && m_hiv.hasLoweredViralLoad()) ? m_hiv.getLastTreatmentStartTime() : infinity;
  
  int aidsDeath = -1;
  if (hasDied())
  {
    aidsDeath = 0;
    if (m_hiv.wasAIDSDeath())
      aidsDeath = 1;
  }
  
  double hsv2InfectionTime = (m_hsv2.isInfected())? m_hsv2.getInfectionTime() : infinity;
  Person *pHSV2Origin = (m_hsv2.isInfected()) ? m_hsv2.getInfectionOrigin() : 0;
  int hsv2origin = (pHSV2Origin != 0) ? (int)pHSV2Origin->getPersonID() : (-1); // TODO: cast should be ok
  
  double cd4AtInfection = (m_hiv.isInfected())?m_hiv.getCD4CountAtInfectionStart() : (-1);
  double cd4AtDeath = (m_hiv.isInfected())?m_hiv.getCD4CountAtDeath() : (-1);

  LogPerson.print("%d,%d,%10.10f,%10.10f,%d,%d,%10.10f,%10.10f,%10.10f,%d,%10.10f,%d,%d,%10.10f,%10.10f,%10.10f,%10.10f,%d,%10.10f,%d,%10.10f,%10.10f,%10.10f,%d,%d",
                  id, gender, timeOfBirth, timeOfDeath, fatherID, motherID, debutTime,
                  formationEagerness,formationEagernessMSM, SexualRole,
                  infectionTime, origin, infectionType, log10SPVLoriginal, treatmentTime,
                  m_location.x, m_location.y, aidsDeath,
                  hsv2InfectionTime, hsv2origin, cd4AtInfection, cd4AtDeath, m_health_seeking_propensity, infectionSite, STIdiagnoses);
}

void Person::writeToGonorrheaTreatLog()
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  
  // Recovery time
  double recoveredTime = (m_gonorrhea.isInfected())? infinity : m_gonorrhea.getRecoveryTime();
  // Diagnosis time
  // double diagTime = (m_gonorrhea.isInfected())? infinity : m_gonorrhea.getDiagnosisTime();
  // Treated?
  bool Treated = (m_gonorrhea.isTreated())? 1 : 0;

  LogGonorrheaTreat.print("%d,%d,%10.10f,%d",
                     id, gender, recoveredTime, Treated);  
    
}

void Person::writeToChlamydiaTreatLog()
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  
  // int diagnosed = (m_gonorrhea.isDiagnosed())?1:0;
  
  // Recovery time
  double recoveredTime = (m_chlamydia.isInfected())? infinity : m_chlamydia.getRecoveryTime();
  // Treated?
  bool Treated = (m_chlamydia.isTreated())? 1 : 0;
  
  // LogChlamydiaTreat.print("%d,%d,%10.10f,%d,%d",
  //                         id, gender, recoveredTime, Treated, diagnosed);  
  LogChlamydiaTreat.print("%d,%d,%10.10f,%d",
                          id, gender, recoveredTime, Treated);  
  
}

void Person::writeToSyphilisTreatLog()
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  
  // Recovery time
  double recoveredTime = (m_syphilis.isInfected())? infinity : m_syphilis.getRecoveryTime();
  // Treated?
  bool Treated = (m_syphilis.isTreated())? 1 : 0;
  
  LogSyphilisTreat.print("%d,%d,%10.10f,%d",
                          id, gender, recoveredTime, Treated);  
  
}

void Person::writeToGonorrheaLog() 
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  int SexualRole = getPreferredSexualRole();
  int diagnosed = (m_gonorrhea.isDiagnosed())?1:0;
  
  // Infection time
  double infectionTime = (m_gonorrhea.isInfected())? m_gonorrhea.getInfectionTime() : infinity;

  int infectionType = 0;
  switch(m_gonorrhea.getInfectionType())
  {
  case Person_Gonorrhea::None:
    infectionType = -1;
    break;
  case Person_Gonorrhea::Seed:
    infectionType = 0;
    break;
  case Person_Gonorrhea::Partner:
    infectionType = 1;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionType = 10000 + (int)m_gonorrhea.getInfectionType();
  }
  
  int infectionSite = 0;
  switch(m_gonorrhea.getInfectionSite())
  {
  case Person_Gonorrhea::Vaginal:
    infectionSite = 0;
    break;
  case Person_Gonorrhea::Rectal:
    infectionSite = 1;
    break;
  case Person_Gonorrhea::Urethral:
    infectionSite = 2;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionSite = 10000 + (int)m_gonorrhea.getInfectionSite();
  }
  
  int diseaseStage = 0;
  switch(m_gonorrhea.getDiseaseStage())
  {
  case Person_Gonorrhea::Susceptible:
    diseaseStage = 0;
    break;
  case Person_Gonorrhea::Asymptomatic:
    diseaseStage = 1;
    break;
  case Person_Gonorrhea::Symptomatic:
    diseaseStage = 2;
    break;
  default:
    diseaseStage = 1000 + (int)m_gonorrhea.getDiseaseStage();
  }
  
  Person *pNGOrigin = (m_gonorrhea.isInfected()) ? m_gonorrhea.getInfectionOrigin() : 0;
  int OrigId = (pNGOrigin != 0) ? (int)pNGOrigin->getPersonID() : (-1); // TODO: cast should be ok
  // type of relationship
  // int RelType = 0; // heterosexual
  // if(pNGOrigin->isMan() && isMan()){
  //   RelType = 1; // MSM
  // }
  
  LogGonorrhea.print("%d,%d,%d,%d,%10.10f,%d,%d,%d,%d",
                     id, OrigId, gender, SexualRole, infectionTime, infectionType, infectionSite, diseaseStage, diagnosed);
}

void Person::writeToChlamydiaLog() 
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  int SexualRole = getPreferredSexualRole();
  int diagnosed = (m_chlamydia.isDiagnosed())?1:0;
  
  // Infection time
  double infectionTime = (m_chlamydia.isInfected())? m_chlamydia.getInfectionTime() : infinity;
  
  int infectionType = 0;
  switch(m_chlamydia.getInfectionType())
  {
  case Person_Chlamydia::None:
    infectionType = -1;
    break;
  case Person_Chlamydia::Seed:
    infectionType = 0;
    break;
  case Person_Chlamydia::Partner:
    infectionType = 1;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionType = 10000 + (int)m_chlamydia.getInfectionType();
  }
  
  int infectionSite = 0;
  switch(m_chlamydia.getInfectionSite())
  {
  case Person_Chlamydia::Vaginal:
    infectionSite = 0;
    break;
  case Person_Chlamydia::Rectal:
    infectionSite = 1;
    break;
  case Person_Chlamydia::Urethral:
    infectionSite = 2;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionSite = 10000 + (int)m_chlamydia.getInfectionSite();
  }
  
  int diseaseStage = 0;
  switch(m_chlamydia.getDiseaseStage())
  {
  case Person_Chlamydia::Susceptible:
    diseaseStage = 0;
    break;
  case Person_Chlamydia::Asymptomatic:
    diseaseStage = 1;
    break;
  case Person_Chlamydia::Symptomatic:
    diseaseStage = 2;
    break;
  default:
    diseaseStage = 1000 + (int)m_chlamydia.getDiseaseStage();
  }
  
  Person *pNGOrigin = (m_chlamydia.isInfected()) ? m_chlamydia.getInfectionOrigin() : 0;
  int OrigId = (pNGOrigin != 0) ? (int)pNGOrigin->getPersonID() : (-1); // TODO: cast should be ok
  // type of relationship
  // int RelType = 0; // heterosexual
  // if(pNGOrigin->isMan() && isMan()){
  //   RelType = 1; // MSM
  // }
  
  LogChlamydia.print("%d,%d,%d,%d,%10.10f,%d,%d,%d,%d",
                     id, OrigId, gender, SexualRole, infectionTime, infectionType, infectionSite, diseaseStage, diagnosed);
}

void Person::writeToSyphilisLog() 
{
  double infinity = std::numeric_limits<double>::infinity();
  double NaN = std::numeric_limits<double>::quiet_NaN();
  
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  
  int gender = (isMan())?0:1;
  int SexualRole = getPreferredSexualRole();
  int diagnosed = (m_syphilis.isDiagnosed())?1:0;
  
  // Infection time
  double infectionTime = (m_syphilis.isInfected())? m_syphilis.getInfectionTime() : infinity;
  
  int infectionType = 0;
  switch(m_syphilis.getInfectionType())
  {
  case Person_Syphilis::None:
    infectionType = -1;
    break;
  case Person_Syphilis::Seed:
    infectionType = 0;
    break;
  case Person_Syphilis::Partner:
    infectionType = 1;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionType = 10000 + (int)m_syphilis.getInfectionType();
  }
  
  int infectionSite = 0;
  switch(m_syphilis.getInfectionSite())
  {
  case Person_Syphilis::Vaginal:
    infectionSite = 0;
    break;
  case Person_Syphilis::Rectal:
    infectionSite = 1;
    break;
  case Person_Syphilis::Urethral:
    infectionSite = 2;
    break;
  default: // Unknown, but don't abort the program at this point
    infectionSite = 10000 + (int)m_syphilis.getInfectionSite();
  }
  
  int diseaseStage = 0;
  switch(m_syphilis.getDiseaseStage())
  {
  case Person_Syphilis::Susceptible:
    diseaseStage = 0;
    break;
  case Person_Syphilis::Primary:
    diseaseStage = 1;
    break;
  case Person_Syphilis::Secondary:
    diseaseStage = 2;
    break;
  case Person_Syphilis::Latent:
    diseaseStage = 3;
    break;
  default:
    diseaseStage = 1000 + (int)m_syphilis.getDiseaseStage();
  }
  
  Person *pNGOrigin = (m_syphilis.isInfected()) ? m_syphilis.getInfectionOrigin() : 0;
  int OrigId = (pNGOrigin != 0) ? (int)pNGOrigin->getPersonID() : (-1); // TODO: cast should be ok
  // type of relationship
  // int RelType = 0; // heterosexual
  // if(pNGOrigin->isMan() && isMan()){
  //   RelType = 1; // MSM
  // }
  
  LogSyphilis.print("%d,%d,%d,%d,%10.10f,%d,%d,%d,%d",
                     id, OrigId, gender, SexualRole, infectionTime, infectionType, infectionSite, diseaseStage, diagnosed);
}

void Person::writeToLocationLog(double tNow)
{
  LogLocation.print("%10.10f,%d,%10.10f,%10.10f", tNow, (int)getPersonID(), m_location.x, m_location.y);
}

void Person::writeToTreatmentLog(double dropoutTime, bool justDied)
{
  int id = (int)getPersonID(); // TODO: should fit in an 'int' (easier for output)
  int gender = (isMan())?0:1;
  int justDiedInt = (justDied)?1:0;
  double lastTreatmentStartTime = m_hiv.getLastTreatmentStartTime();
  double lastCD4AtDiagnosis = m_hiv.getLastCD4CountAtDiagnosis();
  double lastCD4AtTreatmentStart = m_hiv.getLastCD4CountAtARTStart();
  
  assert(m_hiv.hasLoweredViralLoad());
  assert(lastTreatmentStartTime >= 0);
  
  LogTreatment.print("%d,%d,%10.10f,%10.10f,%d,%10.10f,%10.10f", id, gender, lastTreatmentStartTime, 
                     dropoutTime, justDiedInt, lastCD4AtDiagnosis, lastCD4AtTreatmentStart);
}

Man::Man(double dateOfBirth) : Person(dateOfBirth, Male)
{
}

Man::~Man()
{
}

Woman::Woman(double dateOfBirth) : Person(dateOfBirth, Female)
{
  m_pregnant = false;
}

Woman::~Woman()
{
}

ConfigFunctions personConfigFunctions(Person::processConfig, Person::obtainConfig, "Person");

