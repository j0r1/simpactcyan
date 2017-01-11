#ifndef SIMPACTBINDINGS_H

#define SIMPACTBINDINGS_H

#include <Python.h>
#include "populationinterfaces.h"
#include "populationevent.h"
#include "personbase.h"
#include <assert.h>
#include <iostream>

class PersonCXX : public PersonBase
{
public:
	PersonCXX(double dateOfBirth, bool man);
	~PersonCXX();

	void setPythonObject(PyObject *pObj);
	PyObject *getPythonObject();

	bool isMan() const { return (getGender() == Male)?true:false; }
	bool isWoman() const { return (getGender() == Female)?true:false; }
private:
	PyObject *m_pObj;
};

class SimpactEventCXX : public PopulationEvent
{
public:
	SimpactEventCXX();
	SimpactEventCXX(PersonCXX *p1);
	SimpactEventCXX(PersonCXX *p1, PersonCXX *p2);
	~SimpactEventCXX();

	void setPythonObject(PyObject *pObj);
	PyObject *getPythonObject();

	void fire(Algorithm *pAlgorithm, State *pState, double t);
	std::string getDescription(double tNow) const;

	int getNumberOfPersons() const { return PopulationEvent::getNumberOfPersons(); }
	PersonCXX *getPerson(int idx) { return static_cast<PersonCXX*>(PopulationEvent::getPerson(idx)); }
	
	bool isEveryoneAffected() const;
	void markOtherAffectedPeople(const PopulationStateInterface &population);
protected:
	bool isUseless(const PopulationStateInterface &population) override;
	double getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState);
	double calculateInternalTimeInterval(const State *pState, double t0, double dt);
	double solveForRealTimeInterval(const State *pState, double Tdiff, double t0);
private:
	mutable PyObject *m_pObj;
};

class SimpactPopulationCXX : public PopulationStateExtra, public PopulationAlgorithmAboutToFireInterface
{
public:
	SimpactPopulationCXX(PopulationAlgorithmInterface *alg, PopulationStateInterface *state, PyObject *pObj);
	~SimpactPopulationCXX();

	PersonCXX **getAllPeople()					{ return reinterpret_cast<PersonCXX**>(m_state.getAllPeople()); }
	PersonCXX **getMen()						{ return reinterpret_cast<PersonCXX**>(m_state.getMen()); }
	PersonCXX **getWomen()						{ return reinterpret_cast<PersonCXX**>(m_state.getWomen()); }
	PersonCXX **getDeceasedPeople()				{ return reinterpret_cast<PersonCXX**>(m_state.getDeceasedPeople()); }

	int getNumberOfPeople() const				{ return m_state.getNumberOfPeople(); }
	int getNumberOfMen() const					{ return m_state.getNumberOfMen(); }
	int getNumberOfWomen() const				{ return m_state.getNumberOfWomen(); }
	int getNumberOfDeceasedPeople() const		{ return m_state.getNumberOfDeceasedPeople(); }

	void addNewPerson(PersonCXX *pPerson)			{ m_state.addNewPerson(pPerson); }
	void setPersonDied(PersonCXX *pPerson)			{ m_state.setPersonDied(pPerson); }
	void markAffectedPerson(PersonCXX *pPerson) const	{ m_state.markAffectedPerson(pPerson); }

	bool_t run(double *tMax, long long *maxEvents) { int64_t maxEvt = (int64_t)(*maxEvents); bool_t r = m_alg.run(*tMax, maxEvt); *maxEvents = (long long)maxEvt; return r; }

	double getTime() const						{ return m_state.getTime(); }
	void onNewEvent(SimpactEventCXX *pEvt)		{ m_alg.onNewEvent(pEvt); }
	GslRandomNumberGenerator *getRandomNumberGenerator() const { return m_alg.getRandomNumberGenerator(); }

	mutable PyObject *m_pObj;
private:
	void onAboutToFire(PopulationEvent *pEvt);

	PopulationStateInterface &m_state;
	PopulationAlgorithmInterface &m_alg;
};

inline SimpactPopulationCXX &SIMPACTPOPULATION(State *pState)
{
	assert(pState != 0);
	PopulationStateInterface &state = static_cast<PopulationStateInterface &>(*pState);
	assert(state.getExtraStateInfo() != 0);
	return static_cast<SimpactPopulationCXX &>(*state.getExtraStateInfo());
}

inline const SimpactPopulationCXX &SIMPACTPOPULATION(const State *pState)
{
	assert(pState != 0);
	const PopulationStateInterface &state = static_cast<const PopulationStateInterface &>(*pState);
	assert(state.getExtraStateInfo() != 0);
	return static_cast<const SimpactPopulationCXX &>(*state.getExtraStateInfo());
}

#endif // SIMPACTBINDINGS_H


