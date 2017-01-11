#ifndef TIMEINTERVALMAPPER_H

#define TIMEINTERVALMAPPER_H

#include "function.h"
#include <assert.h>

// UNDER CONSTRUCTION

class TimeIntervalMapper
{
public:
	TimeIntervalMapper(Function &f) : m_f(f)							{ }
	~TimeIntervalMapper()										{ }

	double mapToInternalTimeInverval(double t0, double dt);
	double mapToRealWorldTimeInterval(double t0, double dT);
private:
	Function &m_f;
};

inline double TimeIntervalMapper::mapToInternalTimeInverval(double t0, double dt)
{
	assert(m_f.hasPrimitive());

	int primType = m_f.getPrimitiveType(t0);
	double dT = m_f.evaluatePrimitive(t0+dt, primType) - m_f.evaluatePrimitive(t0, primType);

	assert(dT >= 0);

	return dT;
}

inline double TimeIntervalMapper::mapToRealWorldTimeInterval(double t0, double dT)
{
	assert(m_f.hasPrimitive());
	assert(m_f.hasInversePrimitive());

	int primType = m_f.getPrimitiveType(t0);
	double Pt0 = m_f.evaluatePrimitive(t0, primType);
	double I = m_f.evaluateInversePrimitive(dT + Pt0, primType);
	double dt = I - t0;

	assert(dt >= 0);

	return dt;
}

#endif // TIMEINTERVALMAPPER_H

