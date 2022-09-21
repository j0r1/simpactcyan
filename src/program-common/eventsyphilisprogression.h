#ifndef EVENTSYPHILISPROGRESSION_H
#define EVENTSYPHILISPROGRESSION_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventSyphilisProgression : public SimpactEvent
{
public:
	EventSyphilisProgression(Person *pPerson);
	~EventSyphilisProgression();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static ProbabilityDistribution *s_pExposedStageDurationDistribution;
	static ProbabilityDistribution *s_pPrimaryStageDurationDistribution;
	static ProbabilityDistribution *s_pSecondaryStageDurationDistribution;
	static ProbabilityDistribution *s_pLatentStageDurationDistribution;
};

#endif // EVENTSYPHILISPROGRESSION_H
