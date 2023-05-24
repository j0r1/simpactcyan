#ifndef EVENTHIVIMPORT_H

#define EVENTHIVIMPORT_H

#include "eventseedbase.h"

class ConfigSettings;

class EventHIVImport: public EventSeedBase
{
  
public:
  EventHIVImport();
  ~EventHIVImport();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);

private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  static double m_seedInterval;
  static int m_importAmount;
  
};

#endif // EVENTHIVIMPORT_H