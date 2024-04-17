#ifndef EVENTHSV2DIAGNOSIS_H
#define EVENTHSV2DIAGNOSIS_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class HazardFunctionHSV2Diagnosis : public HazardFunctionExp
{
public:
  HazardFunctionHSV2Diagnosis(Person *pPerson, double baseline, double diagPartnersFactor, double healthSeekingPropensityFactor, double beta);
  
  double evaluate(double t);
  
private:
  Person *m_pPerson;
  const double m_baseline, m_diagPartnersFactor, m_healthSeekingPropensityFactor;
  const double m_beta;
};

class EventHSV2Diagnosis : public SimpactEvent
{
public:
  EventHSV2Diagnosis(Person *pPerson, bool scheduleImmediately = false);
  ~EventHSV2Diagnosis();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  // Since the hazard depends on the number of diagnosed partners,
  // every partner of this person who is infected (so diagnosis event
  // is possible) needs to be marked as affected
  void markOtherAffectedPeople(const PopulationStateInterface &population);
  static int getD(Person *pPerson);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double calculateInternalTimeInterval(const State *pState, double t0, double dt);
  double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
  static double getTMax(const Person *pPerson);
  bool isUseless(const PopulationStateInterface &population) override;
  
  bool m_scheduleImmediately;
  bool isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen);
  bool partnerWillingToTest(Person *pPerson, GslRandomNumberGenerator *pRndGen);
  
  static double s_baseline;
  static double s_beta;
  static double s_tMax;
  static double s_diagPartnersFactor;
  static double s_healthSeekingPropensityFactor;
  
  static bool s_partnerNotificationEnabled;
  
  
};

#endif // EVENTHSV2DIAGNOSIS_H