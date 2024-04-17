#ifndef EVENTHSV2PROGRESSION_H
#define EVENTHSV2PROGRESSION_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventHSV2Progression : public SimpactEvent
{
public:
  EventHSV2Progression(Person *pPerson);
  ~EventHSV2Progression();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  
  bool isUseless(const PopulationStateInterface &population) override;
  
  static ProbabilityDistribution *s_hPrimaryStageDurationDistribution;
  
};

#endif // EVENTHSV2PROGRESSION_H
