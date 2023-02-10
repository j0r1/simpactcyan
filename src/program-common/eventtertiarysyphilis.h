#ifndef EVENTTERTIARYSYPHILIS_H
#define EVENTTERTIARYSYPHILIS_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventTertiarySyphilis : public SimpactEvent
{
public: 
  EventTertiarySyphilis(Person *pPerson, bool scheduleImmediately = false);
  ~EventTertiarySyphilis();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  bool isUseless(const PopulationStateInterface &population) override;
  bool m_scheduleImmediately;
  
  static ProbabilityDistribution *s_pTertiaryStageTimeDistribution;
  
};

#endif // EVENTTERTIARYSYPHILIS_H