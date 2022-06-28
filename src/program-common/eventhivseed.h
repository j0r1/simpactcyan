#ifndef EVENTHIVSEED_H

#define EVENTHIVSEED_H

#include "eventseedbase.h"

class ConfigSettings;

class HIVSeedEventSettings: public SeedEventSettings
{
public:
	HIVSeedEventSettings();

	double m_fraction_chronic;
	double m_fraction_aids;
	double m_fraction_final_aids;
	double m_fraction_diagnosed;

};

class EventHIVSeed : public EventSeedBase
{
public:
	EventHIVSeed();
	~EventHIVSeed();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);
	
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static double getSeedTime()									{ return s_settings.m_seedTime; }
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static HIVSeedEventSettings s_settings;
};

#endif // EVENTHIVSEED_H

