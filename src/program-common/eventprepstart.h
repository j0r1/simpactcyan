#ifndef EVENTPREPSTART_H
#define EVENTPREPSTART_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;

class EventPrePStart : public SimpactEvent {
public:
	EventPrePStart(Person *pPerson);
	~EventPrePStart();

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
	// TODO age factor?
	// TODO genderfactor / msm factor?
	// TODO health seeking propensity factor
	// TODO (diagnosed) partners factor
	static double s_beta;
	static double s_tMax;
};

class HazardFunctionPrePStart : public HazardFunctionExp
{
public:
	HazardFunctionPrePStart(Person *pPerson, double baseline, double beta);
private:
	Person *m_pPerson;
	const double m_baseline;
	const double m_beta;
};

#endif // EVENTPREPSTART_H

/*
class ProbabilityDistribution;

class EventDiagnosis : public SimpactEvent
{
public:

	// Since the hazard depends on the number of diagnosed partners,
	// every partner of this person who is infected (so diagnosis event
	// is possible) needs to be marked as affected
	void markOtherAffectedPeople(const PopulationStateInterface &population);
};

 */
