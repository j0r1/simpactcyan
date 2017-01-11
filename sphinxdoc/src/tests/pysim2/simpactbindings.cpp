#include "simpactbindings.h"
#include "populationevent.h"
#ifdef NDEBUG
	#include "simpact.h"
#else
	#include "simpactdebug.h"
#endif // NDEBUG
#include "algorithm.h"
#include <iostream>
#include <limits>

using namespace std;

void abortWithMessage(const std::string &msg);

PersonCXX::PersonCXX(double dateOfBirth, bool man) : PersonBase((man)?Male:Female, dateOfBirth) 
{
	m_pObj = 0;
}

PersonCXX::~PersonCXX()
{
	Py_XDECREF(m_pObj);
}

void PersonCXX::setPythonObject(PyObject *pObj)
{
	if (!pObj)
		abortWithMessage("Internal error: object pointer is null");
	if (m_pObj)
		abortWithMessage("Internal error: setting person object again");

	m_pObj = pObj;
	Py_INCREF(m_pObj);
}

PyObject *PersonCXX::getPythonObject()
{
	Py_XINCREF(m_pObj);
	return m_pObj;
}

SimpactPopulationCXX::SimpactPopulationCXX(PopulationAlgorithmInterface *alg, PopulationStateInterface *state, PyObject *pObj) 
	: PopulationStateExtra(), m_state(*state), m_alg(*alg)
{
	m_pObj = pObj;
	m_state.setExtraStateInfo(this);
	m_alg.setAboutToFireAction(this);
}

SimpactPopulationCXX::~SimpactPopulationCXX()
{
//	cout << "~SimpactPopulationCXX" << endl;
	delete &m_state;
	delete &m_alg;
}

void SimpactPopulationCXX::onAboutToFire(PopulationEvent *pEvt)
{
	if (!PyObject_HasAttrString(m_pObj, "onAboutToFire"))
	{
		double t = getTime();
		std::cout << t << "\t" << pEvt->getDescription(t) << std::endl;
	}
	else
	{
		SimpactEventCXX *pSimpactEvent = static_cast<SimpactEventCXX *>(pEvt);
		cy_call_void_event_func(m_pObj, "onAboutToFire", pSimpactEvent);
	}
}

SimpactEventCXX::SimpactEventCXX() : PopulationEvent()
{
	m_pObj = 0;
}

SimpactEventCXX::SimpactEventCXX(PersonCXX *p1) : PopulationEvent(p1)
{
	m_pObj = 0;
}

SimpactEventCXX::SimpactEventCXX(PersonCXX *p1, PersonCXX *p2) : PopulationEvent(p1, p2)
{
	m_pObj = 0;
}

SimpactEventCXX::~SimpactEventCXX()
{
	Py_XDECREF(m_pObj);
}

void SimpactEventCXX::setPythonObject(PyObject *pObj)
{
	if (!pObj)
		abortWithMessage("Internal error: object pointer is null");
	if (m_pObj)
		abortWithMessage("Internal error: setting person object again");

	m_pObj = pObj;
	Py_INCREF(m_pObj);
}

PyObject *SimpactEventCXX::getPythonObject()
{
	Py_XINCREF(m_pObj);
	return m_pObj;
}

void SimpactEventCXX::fire(Algorithm *pAlg, State *pState, double t)
{
	if (!PyObject_HasAttrString(m_pObj, "fire"))
		return;

	SimpactPopulationCXX &pop = SIMPACTPOPULATION(pState);
	string errStr;
	if (!cy_call_bool_object_double_func(m_pObj, "fire", pop.m_pObj, t, &errStr))
		pState->setAbortAlgorithm("Error executing specific event: " + errStr);
}

std::string SimpactEventCXX::getDescription(double tNow) const
{
	if (!m_pObj)
		abortWithMessage("SimpactEventCXX Internal error: no object has been set");

	return cy_call_string_double_func(m_pObj, (char*)"getDescription", tNow);
}

bool SimpactEventCXX::isEveryoneAffected() const
{
	if (!PyObject_HasAttrString(m_pObj, "isEveryoneAffected"))
		return false;

	return cy_call_bool_func(m_pObj, "isEveryoneAffected");
}

void SimpactEventCXX::markOtherAffectedPeople(const PopulationStateInterface &population)
{
	if (!PyObject_HasAttrString(m_pObj, "markOtherAffectedPeople"))
		return;

	const SimpactPopulationCXX *pPop = static_cast<const SimpactPopulationCXX *>(population.getExtraStateInfo());
	cy_call_void_object_func(m_pObj, "markOtherAffectedPeople", pPop->m_pObj);
}

bool SimpactEventCXX::isUseless(const PopulationStateInterface &population)
{
	if (!PyObject_HasAttrString(m_pObj, "isUseless"))
		return false;

	// TODO: also pass the population as an argument here!
	return cy_call_bool_func(m_pObj, "isUseless");
}

double SimpactEventCXX::getNewInternalTimeDifference(GslRandomNumberGenerator *pRndGen, const State *pState)
{
	if (!PyObject_HasAttrString(m_pObj, "getNewInternalTimeDifference"))
		return EventBase::getNewInternalTimeDifference(pRndGen, pState);

	const SimpactPopulationCXX &pop = SIMPACTPOPULATION(pState);
	return cy_call_double_rng_object(m_pObj, "getNewInternalTimeDifference", pRndGen, pop.m_pObj, numeric_limits<double>::quiet_NaN());
}

double SimpactEventCXX::calculateInternalTimeInterval(const State *pState, double t0, double dt)
{
	if (!PyObject_HasAttrString(m_pObj, "calculateInternalTimeInterval"))
		return EventBase::calculateInternalTimeInterval(pState, t0, dt);

	const SimpactPopulationCXX &pop = SIMPACTPOPULATION(pState);
	return cy_call_double_object_double_double(m_pObj, "calculateInternalTimeInterval", pop.m_pObj, t0, dt, numeric_limits<double>::quiet_NaN());
}

double SimpactEventCXX::solveForRealTimeInterval(const State *pState, double Tdiff, double t0)
{
	if (!PyObject_HasAttrString(m_pObj, "solveForRealTimeInterval"))
		return EventBase::solveForRealTimeInterval(pState, Tdiff, t0);

	const SimpactPopulationCXX &pop = SIMPACTPOPULATION(pState);
	return cy_call_double_object_double_double(m_pObj, "solveForRealTimeInterval", pop.m_pObj, Tdiff, t0, numeric_limits<double>::quiet_NaN());
}

