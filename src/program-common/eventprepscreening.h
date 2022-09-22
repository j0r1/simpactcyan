#ifndef EVENTPREPSCREENING_H
#define EVENTPREPSCREENING_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventPrePScreening : public SimpactEvent
{
public:
	EventPrePScreening(Person *pPerson, bool scheduleImmediately = false);
	~EventPrePScreening();

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

	static ProbabilityDistribution *s_pScreeningIntervalDistribution;
};

#endif // EVENTPREPSCREENING_H
