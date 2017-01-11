#ifndef FUNCTION_H

#define FUNCTION_H

// UNDER CONSTRUCTION

#include <limits>

class Function
{
public:
	Function(bool hasPrimitive, bool hasInversePrimitive) : m_hasPrimitive(hasPrimitive), 
	                                                        m_hasInversePrimitive(hasInversePrimitive)	{ }
	virtual ~Function()									{ }
	
	bool hasPrimitive() const								{ return m_hasPrimitive; }
	bool hasInversePrimitive() const							{ return m_hasInversePrimitive; }

	virtual double evaluate(double x) = 0;

	virtual int getPrimitiveType(double x)							{ return 0; }
	virtual double evaluatePrimitive(double x, int pType = 0)				{ return std::numeric_limits<double>::quiet_NaN(); } // generate a NaN by default
	virtual double evaluateInversePrimitive(double x, int pType = 0)			{ return std::numeric_limits<double>::quiet_NaN(); } // generate a NaN by default
private:
	const bool m_hasPrimitive, m_hasInversePrimitive;
};

#endif // FUNCTION_H

