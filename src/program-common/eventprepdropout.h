#ifndef EVENTPREPDROPOUT_H
#define EVENTPREPDROPOUT_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventPrePDropout : public SimpactEvent
{
public:
	EventPrePDropout(Person *pPerson, double prepStartTime);
	~EventPrePDropout();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	
	double m_prepStartTime;

	static ProbabilityDistribution *s_pPrePDropoutDistribution;
};

#endif // EVENTPREPDROPOUT_H
/*



class EventDropout : public SimpactEvent
{



private:

	double m_treatmentStartTime;
};

 */
