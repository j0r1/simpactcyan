#ifndef PERSON_GONORRHEA_H
#define PERSON_GONORRHEA_H

#include "person_sti.h"
#include "util.h"
// #include "simpactevent.h"
#include "uniformdistribution.h"
#include <assert.h>

class Person;
class ConfigSettings;
class ConfigWriter;
class GslRandomNumberGenerator;
class ProbabilityDistribution;


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
  
  bool isInfected() const; //                            {if (m_diseaseStage == Susceptible) return false; return true;}
  bool isInfectious() const; //                          {if (m_diseaseStage == Susceptible) return false; return true;}
  GonorrheaDiseaseStage getDiseaseStage() const             { return m_diseaseStage; }
  double getInfectionTime() const													{ assert(isInfected()); return m_infectionTime; }
  double getRecoveryTime() const                          { return m_recoveryTime; }
  // double getDiagnosisTime() const                         { return m_diagTime; }
  bool isTreated() const                                 { return m_treated; }
  bool isDiagnosed() const                                { return m_diagnosed; }
  
  Person *getInfectionOrigin() const { assert(isInfected()); return m_pInfectionOrigin; }

  void setInfected(double t, Person *pOrigin, InfectionType iType, InfectionSite iSite);
  void progress(double t, bool treatInd);
  void diagnose(double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
protected:
  
  double m_infectionTime;
  double m_recoveryTime;
  bool m_treated;
  bool m_diagnosed;
  // double m_diagTime;
  Person *m_pInfectionOrigin;
  
  GonorrheaDiseaseStage m_diseaseStage;
  
  static double s_fractionMenSymptomaticRectal;
  static double s_fractionMenSymptomaticUrethral;
  static double s_fractionWomenSymptomatic;

  static UniformDistribution s_uniformDistribution;
};

#endif // PERSON_GONORRHEA_H