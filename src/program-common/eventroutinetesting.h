#ifndef EVENTROUTINETESTING_H
#define EVENTROUTINETESTING_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventRoutineTesting : public SimpactEvent
{
public:
  EventRoutineTesting(Person *pPerson, bool scheduleImmediately = false);
  ~EventRoutineTesting();
  
  std::string getDescription(double tNow) const;
  void writeLogs(const SimpactPopulation &pop, double tNow) const;
  void fire(Algorithm *pAlgorithm, State *pState, double t);
  
  static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
  static void obtainConfig(ConfigWriter &config);
  
protected:
  bool isUseless(const PopulationStateInterface &pop);
  
private:
  double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  
  bool m_scheduleImmediately;
  
  static ProbabilityDistribution *s_pRoutineTestingIntervalDistribution;
};

#endif // EVENTROUTINETESTING_H