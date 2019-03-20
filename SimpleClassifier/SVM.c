#include <stdlib.h>
#include <math.h>
#include "DataTypes.h"

const double eps = 0.0001;

double squaredEuclideanDistance(double* x, double* y, int dim);
double dotProduct(double* x, double* y, int dim);

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params)
{
	ClassificationResult x;
	x.testSet.columns = 0;
	return x;
}


// kernel function
double K(double* x, double* y, int dim, SVMParams params)
{
	double w = dotProduct(x, y, dim);
	double res;
	switch (params.kernel)
	{
	case lin:
		res = w + params.c0;
		break;
	case poly:
		res = pow(params.gamma * w + params.c0, params.deg);
		break;
	case rbf:
		res = exp(-params.gamma * squaredEuclideanDistance(x, y, dim));
		break;
	}
	return res;
}

double squaredEuclideanDistance(double* x, double* y, int dim)
{
	double res = 0.0;
	for (int i = 0; i < dim; i++)
	{
		res += (x[i] - y[i]) * (x[i] - y[i]);
	}
	return res;
}


double dotProduct(double* x, double* y, int dim)
{
	double res = 0.0;
	for (int i = 0; i < dim; i++)
	{
		res += x[i] * y[i];
	}
	return res;
}

