#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "DataTypes.h"

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params);
ClassifiedData assingClasses(struct CsvData dataSet, char** classesList, int classesCount, BinaryClassificator* classificators, SVMParams params);
char* FindClass(BinaryClassificator* classificators, char** classesList, int classesCount, double* vector, SVMParams params);
void allocateResultMemory(struct CsvData dataSet, ClassifiedData* result);
void freeMemory(BinaryClassificator* classificators, char** classesList, int classesCount);
char** resolveClassesList(struct CsvData data, int* classesCount);
BinaryClassificator* BuildClassificators(struct CsvData trainSet, char** classesList, int classesCount, SVMParams  params);
BinaryClassificator BuildBinaryClassificator(double** x, int* y, int vectorsCount, int dim, SVMParams  params);
double alfajNewValue(double alfajOld, double yj, double Ei, double Ej, double eta, double L, double H);
double alfaiNewValue(BinaryClassificator classificator, double alfajNew, int i, int j);
double bNewValue(BinaryClassificator bc, double alfaiNew, double alfajNew, double Ei, double Ej, int i, int j, SVMParams params);
double CalculateEta(double* xi, double* xj, int vectorsCount, SVMParams params);
void CalculateBoundaries(BinaryClassificator classificator, int i, int j, double C, double* L, double* H);
int RandomIndex(int i, int n);
double E(BinaryClassificator classificator, SVMParams  params, int j);
double f(BinaryClassificator classificator, double* vector, SVMParams  params);
double K(double* x, double* y, int dim, SVMParams params);
double dotProduct(double* x, double* y, int dim);
double norm1(double* x, double* y, int dim);
double norm2(double* x, double* y, int dim);
SVMParams DefaultParams(int dim);

const double maxIteration = 20;// acceptable number of iterations without changes 

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params)
{
	int classesCount;
	char** classesList = resolveClassesList(trainSet, &classesCount);
	BinaryClassificator* classificators = BuildClassificators(trainSet, classesList, classesCount, params);
	ClassificationResult result;
	result.trainSet = assingClasses(trainSet, classesList, classesCount, classificators, params);
	result.testSet = assingClasses(testSet, classesList, classesCount, classificators, params);
	freeMemory(classificators, classesList, classesCount);
	return result;
}

ClassifiedData assingClasses(struct CsvData dataSet, char** classesList, int classesCount, BinaryClassificator* classificators, SVMParams params)
{
	ClassifiedData result;
	allocateResultMemory(dataSet, &result);

	for (int i = 0; i < result.rows; i++)
	{
		char* classAssigned = FindClass(classificators, classesList, classesCount, result.data[i], params);
		size_t classNameLength = strlen(classAssigned) + 1;
		result.assignedClasses[i] = (char*)malloc(sizeof(char) * classNameLength);
		strcpy(result.assignedClasses[i], classAssigned);
	}
	return result;
}

char* FindClass(BinaryClassificator* classificators, char** classesList, int classesCount, double* vector, SVMParams params)
{
	char* selectedClass = NULL;
	double maxVal = -DBL_MAX;
	for (int i = 0; i < classesCount; i++)
	{
		double value = f(classificators[i], vector, params);
		if (value > maxVal)
		{
			selectedClass = classesList[i];
			maxVal = value;
		}
	}
	return selectedClass;
}

void allocateResultMemory(struct CsvData dataSet, ClassifiedData* result)
{
	result->columns = dataSet.columns;
	result->rows = dataSet.rows;
	result->classes = (char**)malloc(sizeof(char*) * result->rows);
	result->assignedClasses = (char**)malloc(sizeof(char*) * result->rows);
	result->data = (double**)malloc(sizeof(double*) * result->rows);
	result->headers = (char**)malloc(sizeof(char*) * result->columns);
	for (int i = 0; i < result->columns; i++)
	{
		size_t length = strlen(dataSet.headers[i]) + 1;
		result->headers[i] = (char*)malloc(sizeof(char) * length);
		strcpy(result->headers[i], dataSet.headers[i]);
	}

	for (int i = 0; i < result->rows; i++)
	{
		result->data[i] = (double*)malloc(sizeof(double) * result->columns);
		for (int j = 0; j < result->columns; j++)
			result->data[i][j] = dataSet.data[i][j];

		size_t classNameLength = strlen(dataSet.classes[i]) + 1;
		result->classes[i] = (char*)malloc(sizeof(char) * classNameLength);
		strcpy(result->classes[i], dataSet.classes[i]);
	}
}

void freeMemory(BinaryClassificator* classificators, char** classesList, int classesCount)
{
	for (int i = 0; i < classesCount; i++)
	{
		free(classesList[i]);
	}
	free(classesList);

	for (int i = 0; i < classesCount; i++)
	{
		free(classificators[i].alfa);
		free(classificators[i].y);

		for (int j = 0; j < classificators[i].vectorsCount; j++)
			free(classificators[i].x[j]);
		free(classificators[i].x);
	}
	free(classificators);
}

//get distinct classes values
char** resolveClassesList(struct CsvData data, int* classesCount)
{
	*classesCount = 0;
	char** classesList = NULL;
	for (int i = 0; i < (data.rows); i++)
	{
		int expand = 1;
		if (*classesCount != 0)
		{
			for (int j = 0; j < *classesCount; j++)
			{
				if (strcmp(classesList[j], data.classes[i]) == 0)
				{
					expand = 0;
					break;
				}
			}
		}

		if (expand)
		{
			classesList = (char**)realloc(classesList, sizeof(char*) * ((*classesCount) + 1));
			size_t classNameLength = strlen(data.classes[i]) + 1;
			classesList[*classesCount] = (char *)malloc(sizeof(char) * classNameLength);
			strcpy(classesList[*classesCount], data.classes[i]);
			(*classesCount)++;
		}
	}
	return classesList;
}

BinaryClassificator* BuildClassificators(struct CsvData trainSet, char** classesList, int classesCount, SVMParams  params)
{
	srand(time(NULL));
	BinaryClassificator* classificators = (BinaryClassificator*)malloc(sizeof(BinaryClassificator) * classesCount);

	for (int i = 0; i < classesCount; i++)
	{
		double** x = (double**)malloc(sizeof(double*) * trainSet.rows);
		int* y = (int*)malloc(sizeof(int) * trainSet.rows);
		for (int j = 0; j < trainSet.rows; j++)
		{
			x[j] = (double*)malloc(sizeof(double) * trainSet.columns);
			for (int k = 0; k < trainSet.columns; k++)
				x[j][k] = trainSet.data[j][k];

			if (strcmp(trainSet.classes[j], classesList[i]) == 0)
				y[j] = 1;
			else
				y[j] = -1;
		}


		classificators[i] = BuildBinaryClassificator(x, y, trainSet.rows, trainSet.columns, params);
	}
	return classificators;
}

BinaryClassificator BuildBinaryClassificator(double** x, int* y, int vectorsCount, int dim, SVMParams  params)
{
	BinaryClassificator result;
	result.x = x;
	result.y = y;
	result.alfa = (double*)malloc(sizeof(double) * vectorsCount);
	for (int i = 0; i < vectorsCount; i++)
		result.alfa[i] = 0.0;
	result.b = 0.0;
	result.vectorsCount = vectorsCount;
	result.dim = dim;

	int unchangedIterations = 0;
	while (unchangedIterations < maxIteration)
	{
		int updated = 0;
		for (int i = 0; i < vectorsCount; i++)
		{
			double Ei = E(result, params, i);
			double yE = y[i] * Ei;

			if ((yE < -params.tol && result.alfa[i] < params.c) || (yE > params.tol && result.alfa[i] > 0))
			{
				int j = RandomIndex(i, vectorsCount);
				double Ej = E(result, params, j);
				double L, H;
				CalculateBoundaries(result, i, j, params.c, &L, &H);
				if (fabs(L - H) < params.tol)// if boundaries too close , go to next
					continue;

				double eta = CalculateEta(x[i], x[j], result.dim, params);
				if (eta >= 0)
					continue;
				double alfajNew = alfajNewValue(result.alfa[j], y[j], Ei, Ej, eta, L, H);

				if (fabs(alfajNew - result.alfa[j]) < params.tol)
					continue;
				double alfaiNew = alfaiNewValue(result, alfajNew, i, j);
				double bNew = bNewValue(result, alfaiNew, alfajNew, Ei, Ej, i, j, params);
				//update factor values
				result.alfa[i] = alfaiNew;
				result.alfa[j] = alfajNew;
				result.b = bNew;

				updated++;
			}
		}

		if (updated > 0)
			unchangedIterations = 0;
		else
			unchangedIterations++;
	}
	int podp = 0;
	for (int k = 0; k < vectorsCount; k++)
	{
		if (result.alfa[k] != 0)
			podp++;
	}

	return result;
}

double alfajNewValue(double alfajOld, double yj, double Ei, double Ej, double eta, double L, double H)
{
	double alfaj = alfajOld - (yj *(Ei - Ej) / eta);
	//clipping
	if (alfaj > H)
		alfaj = H;
	else if (alfaj < L)
		alfaj = L;
	return alfaj;
}

double alfaiNewValue(BinaryClassificator classificator, double alfajNew, int i, int j)
{
	double alfai = classificator.alfa[i] + classificator.y[i] * classificator.y[j] * (classificator.alfa[j] - alfajNew);
	return alfai;
}

double bNewValue(BinaryClassificator bc, double alfaiNew, double alfajNew, double Ei, double Ej, int i, int j, SVMParams params)
{
	double b;
	double b1 = bc.b - Ei - bc.y[i] * (alfaiNew - bc.alfa[i])*K(bc.x[i], bc.x[i], bc.dim, params) - bc.y[j] * (alfajNew - bc.alfa[j])*K(bc.x[i], bc.x[j], bc.dim, params);
	double b2 = bc.b - Ej - bc.y[i] * (alfaiNew - bc.alfa[i])*K(bc.x[i], bc.x[j], bc.dim, params) - bc.y[j] * (alfajNew - bc.alfa[j])*K(bc.x[j], bc.x[j], bc.dim, params);
	if (0 < alfaiNew && alfaiNew < params.c)
		b = b1;
	else if (0 < alfajNew && alfajNew < params.c)
		b = b2;
	else
		b = (b1 + b2) / 2;
	return b;
}

double CalculateEta(double* xi, double* xj, int vectorsCount, SVMParams params)
{
	double eta = 2 * K(xi, xj, vectorsCount, params) - K(xi, xi, vectorsCount, params) - K(xj, xj, vectorsCount, params);
	return eta;
}

void CalculateBoundaries(BinaryClassificator classificator, int i, int j, double C, double* L, double* H)
{
	if (classificator.y[i] != classificator.y[j])
	{
		double alfaDiff = classificator.alfa[j] - classificator.alfa[i];
		if (alfaDiff > 0)
			*L = alfaDiff;
		else
			*L = 0;

		if (C + alfaDiff < C)
			*H = C + alfaDiff;
		else
			*H = C;
	}
	else
	{
		double alfaSum = classificator.alfa[j] + classificator.alfa[i];
		if (alfaSum - C > 0)
			*L = alfaSum - C;
		else
			*L = 0;

		if (alfaSum < C)
			*H = alfaSum;
		else
			*H = C;
	}
}

int RandomIndex(int i, int n)
{
	int ind = rand() % n;
	if (ind == i)
		ind = (ind + 1) % n;
	return ind;
}

double E(BinaryClassificator classificator, SVMParams  params, int j)
{

	double fx = f(classificator, classificator.x[j], params);
	double result = fx - classificator.y[j];
	return result;
}

double f(BinaryClassificator classificator, double* vector, SVMParams  params)
{
	double result = 0.0;
	for (int i = 0; i < classificator.vectorsCount; i++)
	{
		double summand = classificator.alfa[i] * (classificator.y[i]) * K(classificator.x[i], vector, classificator.dim, params);
		result += summand;
	}
	result += classificator.b;
	return result;
}

// kernel function
double K(double* x, double* y, int dim, SVMParams params)
{
	double res;
	switch (params.kernel)
	{
	case lin:
		res = dotProduct(x, y, dim) + params.c0;
		break;
	case poly:
		res = pow(params.gamma * dotProduct(x, y, dim) + params.c0, params.deg);
		break;
	case rbf:
		res = exp(-params.gamma * norm2(x, y, dim));
		break;
	case sinc:
	{
		double w = norm1(x, y, dim);
		if (w == 0)
			res = 1;
		else
			res = sin(w) / w;
		break;
	}
	case cauchy:
		res = 1.0 / (1.0 + (norm2(x, y, dim) / (params.c0*params.c0)));
		break;
	case multiquadratic:
		res = -sqrt(norm2(x, y, dim) + params.c0*params.c0);
		break;
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

double norm1(double* x, double* y, int dim)
{
	double res = 0.0;
	for (int i = 0; i < dim; i++)
	{
		res += fabs(x[i] - y[i]);
	}
	return res;
}

double norm2(double* x, double* y, int dim)
{
	double res = 0.0;
	for (int i = 0; i < dim; i++)
	{
		res += (x[i] - y[i]) * (x[i] - y[i]);
	}
	return res;
}
//default svm params, used when other values not specified
SVMParams DefaultParams(int dim) {
	SVMParams params;
	params.c = 1.0;
	params.gamma = 1.0 / (double)dim;
	params.kernel = rbf;
	params.c0 = 0.0;
	params.tol = 0.001;
	params.deg = 2;
	return params;
}
