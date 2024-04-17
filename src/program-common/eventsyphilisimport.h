#ifndef EVENTSYPHILISIMPORT_H

#define EVENTSYPHILISIMPORT_H

#include "eventseedbase.h"

class ConfigSettings;

class EventSyphilisImport: public EventSeedBase
{
  
public:
  EventSyphilisImport();
  ~EventSyphilisImport();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  static double m_syphilisSeedInterval;
  static int m_syphilisImportAmount;
  
};

#endif // EVENTSYPHILISIMPORT_H