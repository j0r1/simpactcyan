#ifndef EVENTCHLAMYDIASEED_H
#define EVENTCHLAMYDIASEED_H

#include "eventseedbase.h"

class ConfigSettings;

class EventChlamydiaSeed : public EventSeedBase
{
public:
	EventChlamydiaSeed();
	~EventChlamydiaSeed();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);

	static double getSeedTime() { return s_settings.m_seedTime; }

private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static SeedEventSettings s_settings;

};

#endif // EVENTCHLAMYDIASEED_H
