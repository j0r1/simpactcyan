#ifndef EVENTDEBUT_H

#define EVENTDEBUT_H

#include "simpactevent.h"

class ConfigSettings;

class EventDebut : public SimpactEvent
{
public:
	EventDebut(Person *pPerson);
	~EventDebut();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);
	
	bool isWillingToRoutineTest(GslRandomNumberGenerator *pRndGen);

	static double getDebutAge()								{ return m_debutAge; }
	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);

	static double m_debutAge;
	static bool m_routineTestingEnabled;
};

#endif // EVENTDEBUT_H
