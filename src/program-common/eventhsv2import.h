#ifndef EVENTHSV2IMPORT_H

#define EVENTHSV2IMPORT_H

#include "eventseedbase.h"

class ConfigSettings;

class EventHSV2Import: public EventSeedBase
{
  
public:
  EventHSV2Import();
  ~EventHSV2Import();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  static double m_hsv2SeedInterval;
  static int m_hsv2ImportAmount;
  
};

#endif // EVENTHSV2IMPORT_H
