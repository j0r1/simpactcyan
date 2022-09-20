#ifndef EVENTCHLAMYDIARECOVERY_H
#define EVENTCHLAMYDIARECOVERY_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;

class EventChlamydiaRecovery : public SimpactEvent
{
public:
	EventChlamydiaRecovery(Person *pPerson);
	~EventChlamydiaRecovery();

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

class HazardFunctionChlamydiaRecovery : public HazardFunctionExp
{
public:
	HazardFunctionChlamydiaRecovery(Person *pPerson, double baseline, double beta);
	~HazardFunctionChlamydiaRecovery() {}
private:
	Person *m_pPerson;

	const double m_baseline;
	const double m_beta;
};

#endif // EVENTCHLAMYDIARECOVERY_H
