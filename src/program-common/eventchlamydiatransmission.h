#ifndef EVENTCHLAMYDIATRANSMISSION_H
#define EVENTCHLAMYDIATRANSMISSION_H

#include "simpactevent.h"
#include "hazardfunctionexp.h"

class ConfigSettings;

class EventChlamydiaTransmission : public SimpactEvent
{
public:
	// Transmission from person1 to person2
	EventChlamydiaTransmission(Person *pPerson1, Person *pPerson2);
	~EventChlamydiaTransmission();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);

	static void infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless(const PopulationStateInterface &population) override;
	double calculateHazardFactor(const SimpactPopulation &population, double t0);

	class HazardFunctionChlamydiaTransmission : public HazardFunctionExp
	{
	public:
		HazardFunctionChlamydiaTransmission(const Person *pPerson1, const Person *pPerson2);
		~HazardFunctionChlamydiaTransmission();

		static double getA(const Person *pPerson1, const Person *pPerson2);
		static double s_b;
	};

	static double getTMax(const Person *pOrigin, const Person *pTarget);

	static double s_a;
	static double s_tMax;
};

#endif // EVENTCHLAMYDIATRANSMISSION_H
