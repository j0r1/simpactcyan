#ifndef EVENTPREPOFFERED_H
#define EVENTPREPOFFERED_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;
class ConfigWriter;

class EventPrePOffered : public SimpactEvent {
public:
	EventPrePOffered(Person *pPerson);
	~EventPrePOffered();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
protected:
	bool isUseless(const PopulationStateInterface &pop);
private:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	static double getTMax(const Person *pPerson);

	static double s_baseline;
	// TODO age factor?
	// TODO genderfactor / msm factor?
	// TODO health seeking propensity factor
	static double s_diagpartnersfactor;
	static double s_beta;
	static double s_tMax;
};

class HazardFunctionPrePOffered : public HazardFunctionExp
{
public:
	HazardFunctionPrePOffered(Person *pPerson, double baseline, double diagpartnersfactor, double beta);
private:
	Person *m_pPerson;
	const double m_baseline;
	const double m_diagpartnersfactor;
	const double m_beta;
};

#endif // EVENTPREPOFFERED_H
