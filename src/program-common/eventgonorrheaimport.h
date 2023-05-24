#ifndef EVENTGONORRHEAIMPORT_H

#define EVENTGONORRHEAIMPORT_H

#include "eventseedbase.h"

class ConfigSettings;

class EventGonorrheaImport: public EventSeedBase
{
  
public:
  EventGonorrheaImport();
  ~EventGonorrheaImport();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  static double m_gonorrheaSeedInterval;
  static int m_gonorrheaImportAmount;
  
};

#endif // EVENTGONORRHEAIMPORT_H