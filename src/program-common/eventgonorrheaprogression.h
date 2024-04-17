#ifndef EVENTGONORRHEAPROGRESSION_H
#define EVENTGONORRHEAPROGRESSION_H

#include "simpactevent.h"

class ConfigSettings;
class ConfigWriter;
class ProbabilityDistribution;

class EventGonorrheaProgression : public SimpactEvent
{
public:
	EventGonorrheaProgression(Person *pPerson);
	~EventGonorrheaProgression();

	std::string getDescription(double tNow) const;
	void writeLogs(const SimpactPopulation &pop, double tNow) const;
	void fire(Algorithm *pAlgorithm, State *pState, double t);

	static void processConfig(ConfigSettings &config, GslRandomNumberGenerator *pRndGen);
	static void obtainConfig(ConfigWriter &config);
private:
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
  bool isUseless(const PopulationStateInterface &population) override;
  
  bool isWillingToTreatSTI(double t, GslRandomNumberGenerator *pRndGen);
  
	static ProbabilityDistribution *s_pInfectionDurationDistributionRectal;
	static ProbabilityDistribution *s_pInfectionDurationDistributionUrethral;
	static ProbabilityDistribution *s_pInfectionDurationDistributionVaginal; 
	
	static ProbabilityDistribution *s_pTimeToSymptoms;
};

#endif // EVENTGONORRHEAPROGRESSION_H
