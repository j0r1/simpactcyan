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
	static int getH(const Person *pPerson1);
	static int getR(const Person *pPerson1, const Person *pPerson2);
	static int getC(const Person *pPerson1, const Person *pPerson2);
	static int getW(const Person *pPerson1);
	
	static double s_a; // baseline
	static double s_tMax;
	static double s_d1; // increase if infectious partner is HIV+
	static double s_d2; // increase if susceptible partner is HIV+
	static double s_f; // increase when susceptible is receptive partner
	static double s_h; // condom use
	static double s_w; // female susceptible
};

#endif // EVENTCHLAMYDIATRANSMISSION_H