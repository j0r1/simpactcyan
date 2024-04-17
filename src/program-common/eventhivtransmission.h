#ifndef EVENTHIVTRANSMISSION_H

#define EVENTHIVTRANSMISSION_H

#include "simpactevent.h"

class ConfigSettings;

class EventHIVTransmission : public SimpactEvent
{
public:
	// Transmission from person1 onto person2
	EventHIVTransmission(Person *pPerson1, Person *pPerson2);
	~EventHIVTransmission();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;

	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
	static double getParamB()																		{ return s_b; }
	static double getParamC()																		{ return s_c; }

	static void infectPerson(SimpactPopulation &population, Person *pOrigin, Person *pTarget, double t, bool schedulAll=true);
protected:
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
	bool isUseless(const PopulationStateInterface &population) override;
	double calculateHazardFactor(const SimpactPopulation &population, double t0);

	static double s_a; 		/// Baseline
	static double s_b; 		/// Specifies influence of current viral load (together with c)
	static double s_c; 		/// Specified influence of current viral load (together with b)
	static double s_d1; 	/// Weight of number of partners of infectious person
	static double s_d2; 	/// Weight of number of partners of susceptible person
	static double s_e1;		/// Influence of co-infection ulcerative STI in (HIV-)infectious person
	static double s_e2;		/// Influence of co-infection ulcerative STI in (HIV-)susceptible person
	static double s_e3;		/// Influence of co-infection non-ulcerative STI in (HIV-)infectious person
	static double s_e4;		/// Influence of co-infection non-ulcerative STI in (HIV-)susceptible person
	static double s_f1;		/// Influence of woman's age on susceptibility
	static double s_f2;		/// Influence of woman's age on susceptibility
	static double s_g1;		/// Set to 1 to include influence of individual susceptibility to HIV and HSV2
	static double s_g2;		/// Set to 1 to include influence of individual susceptibility to HIV only
	static double s_h;		/// Influence of condom use
	static double s_i; 		/// Impact of PreP use of susceptible person.
	static double s_r;    /// impact of the susceptible being the receptive partner (for MSM)
	static double s_tMaxAgeRefDiff;
	static double s_undetectableVL;

	static int getH1(const Person *pPerson);
	static int getH2(const Person *pPerson);
	static int getR(const Person *pPerson1, const Person *pPerson2);
};

#endif // EVENTHIVTRANSMISSION_H
