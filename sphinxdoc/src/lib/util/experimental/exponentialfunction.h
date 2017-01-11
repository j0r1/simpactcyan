#ifndef EXPONENTIALFUNCTION_H

#define EXPONENTIALFUNCTION_H

#include "function.h"

// UNDER CONSTRUCTION

class ExponentialFunction : public Function
{
public:
	// exp(a+b*x)
	ExponentialFunction(double a, double b);
	// exp(a+b*x+c*x*x)
	ExponentialFunction(double a, double b, double c);
	~ExponentialFunction();

	double evaluate(double x);
	int getPrimitiveType(double x);
	double evaluatePrimitive(double x, int pType);
	double evaluateInversePrimitive(double x, int pType);
private:
	bool m_hasSquareTerm;
	double m_a, m_b, m_c;
	double m_offset, m_prefactor, m_prefactor2;
	bool m_cNeg;
};

#endif // EXPONENTIALFUNCTION_H
