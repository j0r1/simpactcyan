#ifndef EVENTGONORRHEARECOVERY_H
#define EVENTGONORRHEARECOVERY_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;

class EventGonorrheaRecovery : public SimpactEvent
{
public:
	EventGonorrheaRecovery(Person *pPerson);
	~EventGonorrheaRecovery();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	static double getTMax(const Person *pPerson);

	static double s_baseline;
	static double s_beta;
	static double s_tMax;
	// TODO health seeking propensity?
	// TODO symptomatic?
};

class HazardFunctionGonorrheaRecovery : public HazardFunctionExp
{
public:
	HazardFunctionGonorrheaRecovery(Person *pPerson, double baseline, double beta);
	~HazardFunctionGonorrheaRecovery() {}
private:
	Person *m_pPerson;

	const double m_baseline;
	const double m_beta;
};

#endif // EVENTGONORRHEARECOVERY_H
