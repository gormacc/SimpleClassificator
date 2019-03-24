#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "DataTypes.h"

const double eps = 0.0001;// for comparing float-pointing numbers

double squaredEuclideanDistance(double* x, double* y, int dim);
double dotProduct(double* x, double* y);
char** resolveClassesList(struct CsvData data);
void allocateResultMemory(struct CsvData dataSet, ClassifiedData* result);
ClassifiedData assingClasses(struct CsvData dataSet, Classificator* classificators, char** classesList);
char * FindClass(double* vector, Classificator* classificators, char** classesList);
Classificator* BuildClassificators(struct CsvData trainSet, SVMParams  params, char** classesList);
Classificator BuildBinaryClassificator(double** vectors, int* belongs, SVMParams  params);

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params)
{
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
		double** vectors = (double**)malloc(sizeof(double*) * trainSet.rows);
		int* belongs = (int*)malloc(sizeof(int) * trainSet.rows);
		for (int j = 0; j < classesCount; j++)
		{
			vectors[j] = (double*)malloc(sizeof(double) * trainSet.columns);
			for (int k = 0; k < classesCount; k++)
				vectors[j][k] = trainSet.data[j][k];

			if (strcmp(trainSet.classes[j], classesList[i]) == 0)
				belongs[j] = 1;
			else
				belongs[j] = -1;
		}

		classificators[i] = BuildBinaryClassificator(vectors, belongs, params);
	}
	return classificators;
}

Classificator BuildBinaryClassificator(double** vectors, int* belongs, SVMParams  params)
{



	//TODO
	Classificator c;
	c.b = 0;
	return c;
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
double K(double* x, double* y, int dim, SVMParams params)
{
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

