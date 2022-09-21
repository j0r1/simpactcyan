#ifndef EVENTSYPHILISSEED_H
#define EVENTSYPHILISSEED_H

#include "eventseedbase.h"

class ConfigSettings;

class EventSyphilisSeed : public EventSeedBase
{
public:
	EventSyphilisSeed();
	~EventSyphilisSeed();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static SeedEventSettings s_settings;
};

#endif // EVENTSYPHILISSEED_H
