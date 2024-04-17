#ifndef EVENTCHLAMYIDYAIMPORT_H
#define EVENTCHLAMYIDYAIMPORT_H

#include "eventseedbase.h"

class ConfigSettings;

class EventChlamydiaImport: public EventSeedBase
{
  
public:
  EventChlamydiaImport();
  ~EventChlamydiaImport();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  static double m_chlamydiaSeedInterval;
  static int m_chlamydiaImportAmount;
  
};

#endif // EVENTCHLAMYIDYAIMPORT_H
