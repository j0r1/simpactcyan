#ifndef EVENTHSV2RECURRENCE_H
#define EVENTHSV2RECURRENCE_H

#include "simpactevent.h"

class ConfigSettings;

class EventHSV2Recurrence : public SimpactEvent
{
  public:
    EventHSV2Recurrence(Person *pPerson, bool scheduleImmediately = false);
    ~EventHSV2Recurrence();
    
    std::string getDescription(double tNow) const;
    void writeLogs(const SimpactPopulation &pop, double tNow) const;
    
    void fire(Algorithm *pAlgorithm, State *pState, double t);
    
    static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
    static void obtainConfig(ConfigWriter &config);
    
  private:
    double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
    
    bool m_scheduleImmediately;
    bool isWillingToTreatSTI(GslRandomNumberGenerator *pRndGen);
    
    static double m_sheddingFreq;
    static double m_nCycles;
    
    static double m_chronicHIV;
    static double m_AIDS;
    
    static double m_redTreat;
};                          

#endif // EVENTHSV2RECURRENCE_H
