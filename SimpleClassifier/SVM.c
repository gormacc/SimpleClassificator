#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "DataTypes.h"

const double maxIteration = 10;// acceptable number of iterations without changes 

char** resolveClassesList(struct CsvData data);
void allocateResultMemory(struct CsvData dataSet, ClassifiedData* result);
ClassifiedData assingClasses(struct CsvData dataSet, Classificator* classificators, char** classesList);
char * FindClass(double* vector, Classificator* classificators, char** classesList);
Classificator* BuildClassificators(struct CsvData trainSet, SVMParams  params, char** classesList);
Classificator BuildBinaryClassificator(double** vectors, int* belongs, SVMParams  params);

double E(double* alfa, double b, double** x, int* y, SVMParams  params, int j);
double f(double* alfa, double b, double** x, int* y, SVMParams  params, int j);
void CalculateBoundaries(double* alfa, int* y, int i, int j, double C, double* L, double* H);
double CalculateEta(double* xi, double* xj, SVMParams params);
double alfaiNewValue(double alfajNew, double* alfa, int* y, int i, int j);
double alfajNewValue(double alfajOld, double yj, double Ei, double Ej, double eta, double L, double H);
double bNewValue(double bOld, double alfaiNew, double alfajNew, double Ei, double Ej, double** x, int* y, double* alfa, int i, int j, SVMParams params);

double K(double* x, double* y, SVMParams params);
double squaredEuclideanDistance(double* x, double* y, int dim);
double dotProduct(double* x, double* y);
int RandomIndex(int i, int n);

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params)
{
	srand(time(NULL));
	char** classesList = resolveClassesList(trainSet);
	Classificator* classificators = BuildClassificators(trainSet, params, classesList);
	ClassificationResult result;
	result.trainSet = assingClasses(trainSet, classificators, classesList);
	result.testSet = assingClasses(testSet, classificators, classesList);
	return result;
}


ClassifiedData assingClasses(struct CsvData dataSet, Classificator* classificators, char** classesList)
{
	ClassifiedData result;
	allocateResultMemory(dataSet, &result);

	for (int i = 0; i < result.rows; i++)
	{
		char* classAssigned = FindClass(result.data[i], classificators, classesList);
		size_t classNameLength = sizeof(classAssigned) / sizeof(char) + 1;
		result.assignedClasses[i] = (char*)malloc(sizeof(char) * classNameLength);
		strcpy(result.assignedClasses[i], classAssigned);
	}
	return result;
}

char * FindClass(double* vector, Classificator* classificators, char** classesList)
{
	char* selectedClass = NULL;
	size_t classesCount = sizeof(classesList) / sizeof(char*);
	for (int i = 0; i < classesCount; i++)
	{
		double value = dotProduct(vector, classificators[i].w) + classificators[i].b;

		if (value >= 0)
		{
			selectedClass = classesList[i];
			break;
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
		size_t length = sizeof(dataSet.headers[i]) / sizeof(char) + 1;
		result->headers[i] = (char*)malloc(sizeof(char) * length);
		strcpy(result->headers[i], dataSet.headers[i]);
	}

	for (int i = 0; i < result->rows; i++)
	{
		result->data[i] = (double*)malloc(sizeof(double*) * result->columns);
		for (int j = 0; j < result->rows; j++)
			result->data[i][j] = dataSet.data[i][j];

		size_t classNameLength = sizeof(dataSet.classes[i]) / sizeof(char) + 1;
		result->classes[i] = (char*)malloc(sizeof(char) * classNameLength);
		strcpy(result->classes[i], dataSet.classes[i]);
	}
}

Classificator* BuildClassificators(struct CsvData trainSet, SVMParams  params, char** classesList)
{
	size_t classesCount = sizeof(classesList) / sizeof(char*);
	Classificator* classificators = (Classificator*)malloc(sizeof(Classificator) * classesCount);

	for (int i = 0; i < classesCount; i++)
	{
		double** x = (double**)malloc(sizeof(double*) * trainSet.rows);
		int* y = (int*)malloc(sizeof(int) * trainSet.rows);
		for (int j = 0; j < classesCount; j++)
		{
			x[j] = (double*)malloc(sizeof(double) * trainSet.columns);
			for (int k = 0; k < classesCount; k++)
				x[j][k] = trainSet.data[j][k];

			if (strcmp(trainSet.classes[j], classesList[i]) == 0)
				y[j] = 1;
			else
				y[j] = -1;
		}

		classificators[i] = BuildBinaryClassificator(x, y, params);
	}
	return classificators;
}

Classificator BuildBinaryClassificator(double** x, int* y, SVMParams  params)
{
	size_t vectorsCount = sizeof(x) / sizeof(double);

	double* alfa = (double*)malloc(sizeof(double) * vectorsCount);
	double b = 0.0;
	int unchangedIterations = 0;
	while (unchangedIterations < maxIteration)
	{
		int updated = 0;
		for (int i = 0; i < vectorsCount; i++)
		{
			double Ei = E(alfa, b, x, y, params, i);
			double yE = y[i] * Ei;
			if ((yE < -params.tol && alfa[i] < params.c) || (yE > params.tol && alfa[i] > 0))
			{
				int j = RandomIndex(i, vectorsCount);
				double Ej = E(alfa, b, x, y, params, j);
				double L, H;
				CalculateBoundaries(alfa, y, i, j, params.c, &L, &H);
				if (abs(L - H) < params.tol)// if boundaries too close , go to next
					continue;
				double eta = CalculateEta(x[i], x[j], params);
				if (eta >= 0)
					continue;
				double alfajNew = alfajNewValue(alfa[j], y[j], Ei, Ej, eta, L, H);

				if (abs(alfajNew - alfa[j]) < params.tol)
					continue;
				double alfaiNew = alfaiNewValue(alfajNew, alfa, y, i, j);
				double bNew = bNewValue(b, alfaiNew, alfajNew, Ei, Ej, x, y, alfa, i, j, params);
				//update factor values
				alfa[i] = alfaiNew;
				alfa[j] = alfajNew;
				b = bNew;

				updated++;
			}
		}

		if (updated > 0)
			unchangedIterations = 0;
		else
			unchangedIterations = 0;
	}

	//TODO
	Classificator c;
	c.b = 0;
	return c;
}
double bNewValue(double bOld, double alfaiNew, double alfajNew, double Ei, double Ej, double** x, int* y, double* alfa, int i, int j, SVMParams params)
{
	double b;
	double b1 = bOld - Ei - y[i] * (alfaiNew - alfa[i])*K(x[i], x[i], params) - y[j] * (alfajNew - alfa[j])*K(x[i], x[j], params);
	double b2 = bOld - Ej - y[i] * (alfaiNew - alfa[i])*K(x[i], x[j], params) - y[j] * (alfajNew - alfa[j])*K(x[j], x[j], params);
	if (0 < alfaiNew && alfaiNew < params.c && 0 < alfajNew && alfajNew < params.c)
		b = (b1 + b2) / 2;
	else if (0 < alfaiNew && alfaiNew < params.c)
		b = b1;
	else if (0 < alfajNew && alfajNew < params.c)
		b = b2;
	return b;
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

double alfaiNewValue(double alfajNew, double* alfa, int* y, int i, int j)
{
	double alfai = alfa[i] + y[i] * y[j] * (alfajNew - alfa[j]);
	return alfai;
}

double CalculateEta(double* xi, double* xj, SVMParams params)
{
	double eta = 2 * K(xi, xj, params) - K(xi, xi, params) - K(xj, xj, params);
	return eta;
}

void CalculateBoundaries(double* alfa, int* y, int i, int j, double C, double* L, double* H)
{
	if (y[i] == y[j])
	{
		double alfaDiff = alfa[j] - alfa[i];
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
		double alfaSum = alfa[j] + alfa[i];
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

double E(double* alfa, double b, double** x, int* y, SVMParams  params, int j)
{
	double fx = f(alfa, b, x, y, params, j);
	double result = fx - y[j];
	return result;
}

double f(double* alfa, double b, double** x, int* y, SVMParams  params, int j)
{
	size_t vectorsCount = sizeof(x) / sizeof(double);
	double result = 0.0;
	for (int i = 0; i < vectorsCount; i++)
	{
		double summand = alfa[i] * y[i] * K(x[i], x[j], params) + b;
	}
	return result;
}

//get distinct classes values
char** resolveClassesList(struct CsvData data)
{
	char** classesList = NULL;
	for (int i = 0; i < (data.rows); i++)
	{
		int expand = 1;
		size_t classesCount = sizeof(classesList) / sizeof(char*);
		if (classesList != NULL)
		{
			for (int j = 0; j < (data.rows); j++)
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
			classesList = (char**)realloc(classesList, sizeof(char*) * (classesCount + 1));
			size_t classNameLength = sizeof(data.classes[i]) / sizeof(char) + 1;
			classesList[classesCount] = (char *)malloc(sizeof(char) * classNameLength);
			strcpy(classesList[classesCount], data.classes[i]);
		}
	}
	return classesList;
}

// kernel function
double K(double* x, double* y, SVMParams params)
{
	size_t dim = sizeof(x) / sizeof(double);
	double w = dotProduct(x, y);
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


double dotProduct(double* x, double* y)
{
	size_t dim = sizeof(x) / sizeof(double);
	double res = 0.0;
	for (int i = 0; i < dim; i++)
	{
		res += x[i] * y[i];
	}
	return res;
}

