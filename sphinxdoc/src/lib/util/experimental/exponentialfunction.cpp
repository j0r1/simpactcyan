#include "exponentialfunction.h"
#include "Faddeeva.hh"
#include "util.h"
#include "inverseerfi.h"
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_cdf.h>
#include <assert.h>
#include <cmath>
#include <limits>

// UNDER CONSTRUCTION

#define SQRT_PI_OVER_TWO 0.88622692545275801364
#define ONE_OVER_SQRT_TWO 0.70710678118654752440

ExponentialFunction::ExponentialFunction(double a, double b) : Function(true, true)
{
	assert(b != 0);

	m_a = a;
	m_b = b;
	m_hasSquareTerm = false;
	m_c = std::numeric_limits<double>::quiet_NaN(); // NaN

	m_prefactor = 0;
	m_prefactor2 = 0;
	m_offset = 0;
	m_cNeg = false;
}

ExponentialFunction::ExponentialFunction(double a, double b, double c) : Function(true, true)
{
	assert(c != 0);

	m_a = a;
	m_b = b;
	m_c = c;
	m_hasSquareTerm = true;
	m_prefactor2 = std::sqrt(std::abs(c));
	m_prefactor = SQRT_PI_OVER_TWO/m_prefactor2 * std::exp(m_a-m_b*m_b/(4.0*m_c));
	m_offset = m_b/(2.0*m_c);
	m_cNeg = (c < 0)?true:false;
}

ExponentialFunction::~ExponentialFunction()
{
}

double ExponentialFunction::evaluate(double x)
{
	double s = m_a + m_b*x;

	if (m_hasSquareTerm)
		s += m_c*x*x;
	
	return std::exp(s);
}

#define PRIMTYPE_ERF	0
#define PRIMTYPE_ERFC	1
#define PRIMTYPE_ERFC2	2

int ExponentialFunction::getPrimitiveType(double x)
{
	if (m_hasSquareTerm && m_cNeg)
	{
		double x2 = (x+m_offset)*m_prefactor2;
		if (x2 > 3.0)
			return PRIMTYPE_ERFC;
		if (x2 < -3.0)
			return PRIMTYPE_ERFC2;
	}
	return PRIMTYPE_ERF;
}

double ExponentialFunction::evaluatePrimitive(double x, int pType)
{
	double result;

	if (!m_hasSquareTerm)
	{
		result = std::exp(m_a + m_b*x)/m_b;
	}
	else
	{
		if (m_cNeg)
		{
			if (pType == PRIMTYPE_ERF)
				result = m_prefactor*gsl_sf_erf((x+m_offset)*m_prefactor2);
			else if (pType == PRIMTYPE_ERFC)
				result = -m_prefactor*gsl_sf_erfc((x+m_offset)*m_prefactor2);
			else
				result = m_prefactor*gsl_sf_erfc(-(x+m_offset)*m_prefactor2);
		}
		else
			result = m_prefactor*Faddeeva::erfi((x+m_offset)*m_prefactor2);
	}

	return result;
}

inline double inv_erf(double x)
{
	return gsl_cdf_gaussian_Pinv(0.5*(x+1.0),ONE_OVER_SQRT_TWO);
}

inline double inv_erfc(double x)
{
	return gsl_cdf_gaussian_Qinv(0.5*x,ONE_OVER_SQRT_TWO);
}

double ExponentialFunction::evaluateInversePrimitive(double y, int pType)
{
	double result;

	if (!m_hasSquareTerm)
	{
		result = (std::log(y*m_b) - m_a)/m_b;
	}
	else
	{
		if (m_cNeg)
		{
			if (pType == PRIMTYPE_ERF)
				result = inv_erf(y/m_prefactor)/m_prefactor2 - m_offset;
			else if (pType == PRIMTYPE_ERFC)
				result = inv_erfc(-y/m_prefactor)/m_prefactor2 - m_offset;
			else
				result = -inv_erfc(y/m_prefactor)/m_prefactor2 - m_offset;
		}
		else
		{
			// TODO: right now this will alloc/destroy some buffers for gsl. Can we avoid this?
			InverseErfI erfi;

			result = erfi.evaluate(y/m_prefactor)/m_prefactor2 - m_offset;
		}
	}

	return result;
}

