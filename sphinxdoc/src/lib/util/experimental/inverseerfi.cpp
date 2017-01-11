#include "inverseerfi.h"
#include "Faddeeva.hh"
#include "util.h"
#include <gsl/gsl_errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>

using namespace std;

InverseErfI::InverseErfI() : Function(false, false)
{
	const gsl_root_fsolver_type * T = gsl_root_fsolver_brent;
	m_pSolver = gsl_root_fsolver_alloc (T);
}

InverseErfI::~InverseErfI()
{
	gsl_root_fsolver_free(m_pSolver);
}

double InverseErfI::evaluate(double x)
{
	if (x < 0)
		return -evaluatePositive(m_pSolver, -x);
	return evaluatePositive(m_pSolver, x);
}

bool InverseErfI::m_tableInit = false;
const int InverseErfI::numEntries = 16384;
double *InverseErfI::pNormalMapping = 0;
double *InverseErfI::pLogMapping = 0;
double InverseErfI::normalBinWidth = 0;
double InverseErfI::logBinWidth = 0;
const double InverseErfI::maxXNormal = 10.0;
const double InverseErfI::maxXLog = 600.0;

double InverseErfI::erfi_function(double x, void *params)
{
	double *offset = (double *)params;
	double ydiff = Faddeeva::erfi(x) - *offset ;

	return ydiff;
}

double InverseErfI::solveWithGSL(gsl_root_fsolver *pSolver, double y, double x_lo, double x_hi, double tolerance)
{
	int status = 0;
	int iter = 0;
	int max_iter = 100;
	double r = 0;
	gsl_function F;

	F.function = erfi_function;
	F.params = &y;

	gsl_root_fsolver_set (pSolver, &F, x_lo, x_hi);
	//printf ("%5d [%.7f, %.7f] %.7f %.7f\n", iter, x_lo, x_hi, r, x_hi - x_lo);

	do
	{
		iter++;
		status = gsl_root_fsolver_iterate(pSolver);
		r = gsl_root_fsolver_root(pSolver);
		x_lo = gsl_root_fsolver_x_lower(pSolver);
		x_hi = gsl_root_fsolver_x_upper(pSolver);
		status = gsl_root_test_interval (x_lo, x_hi, 0, tolerance);

		//if (status == GSL_SUCCESS)
		//	printf ("Converged:\n");
		if (x_hi == x_lo)
			status = GSL_SUCCESS;

		//printf ("%5d [%.7g, %.7g] %.7g %.7g\n", iter, x_lo, x_hi, r, x_hi - x_lo);

	} while (status == GSL_CONTINUE && iter < max_iter);

	//cerr << "Iterations = " << iter << endl;

	return r;
}

double InverseErfI::estimateFromTable(gsl_root_fsolver *pSolver, double x, double realX, double *pTable, 
		                      double binWidth, bool isLogTable)
{
	int pos = x/binWidth;
	double y1 = 0;
	double y2 = 25;

	if (pos > 0 && pos < numEntries)
		y1 = pTable[pos-1];
	
	if (pos >= 0 && pos < numEntries-1)
		y2 = pTable[pos+1];

	double y = solveWithGSL(pSolver, realX, y1, y2, 1e-14);

	return y;
}

double InverseErfI::evaluatePositive(gsl_root_fsolver *pSolver, double x)
{
	if (!m_tableInit)
		abortWithMessage("InverseErfI: table not initialized");

	double y;

	if (x > maxXNormal)
	{
		double logX = std::log(x);

		if (logX >= maxXLog)
			abortWithMessage("InverseErfI: x is too large");

		y = estimateFromTable(pSolver, logX, x, pLogMapping, logBinWidth, true);

	}
	else
		y = estimateFromTable(pSolver, x, x, pNormalMapping, normalBinWidth, false);

	return y;
}

void InverseErfI::initialize()
{
	if (m_tableInit)
		return;

	InverseErfI helperInstance; //  we just use this to get a solver of the correct type
	gsl_root_fsolver *pSolver = helperInstance.m_pSolver;

	pNormalMapping = new double[numEntries];
	pLogMapping = new double[numEntries];

	normalBinWidth = maxXNormal/(double)numEntries;
	logBinWidth = maxXLog/(double)numEntries;

	for (int i = 0 ; i < numEntries ; i++)
	{
		double x1 = normalBinWidth * (i+1);
		double x2 = logBinWidth * (i+1);
		double y1 = solveWithGSL(pSolver, x1);
		double y2 = solveWithGSL(pSolver, std::exp(x2));

		pNormalMapping[i] = y1;
		pLogMapping[i] = y2;
//		cerr << i << " " << y1 << " " << y2 << endl;
	}

	m_tableInit = true;
}

void InverseErfI::destroy()
{
	if (!m_tableInit)
		return;

	delete [] pNormalMapping;
	delete [] pLogMapping;
	m_tableInit = false;
}

