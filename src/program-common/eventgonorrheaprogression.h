#ifndef EVENTGONORRHEAPROGRESSION_H
#define EVENTGONORRHEAPROGRESSION_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventGonorrheaProgression : public SimpactEvent
{
public:
	EventGonorrheaProgression(Person *pPerson);
	~EventGonorrheaProgression();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static ProbabilityDistribution *s_pSymptomaticInfectionDurationDistribution;
	static ProbabilityDistribution *s_pAsymptomaticInfectionDurationDistribution;
};

#endif // EVENTGONORRHEAPROGRESSION_H
