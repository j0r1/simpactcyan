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
	static double s_numPartnersFactor;
	static double s_healthSeekingPropensityFactor;
	static double s_beta; // Time since becoming eligible
	static double s_tMax;
};

class HazardFunctionPrePOffered : public HazardFunctionExp
{
public:
	HazardFunctionPrePOffered(Person *pPerson, double baseline, double numpartnersfactor, double healthseekingpropensityfactor, double beta);
private:
	Person *m_pPerson;
	const double m_baseline;
	const double m_numPartnersFactor;
	const double m_healthSeekingPropensityFactor;
	const double m_beta;
};

#endif // EVENTPREPOFFERED_H
