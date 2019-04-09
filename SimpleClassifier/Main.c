#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "SVM.h"

#define STRINGBUFF 64

ProgramParams readProgramParams();
CsvData allocCsvData(int rows, int columns);
void freeCsvData(CsvData csvData);
void shuffleCsvData(CsvData* data);
int readFile(char* fileName, CsvData* csvData);
void readOneFile(CsvData* trainData, CsvData* testData, char* fileName, int prop);
void normalizeData(CsvData* data);
int typeOfCrossValidation(char* input, int rows);
void crossValidate(CsvData trainData, int cvk, SVMParams params);
void crossValidateFold(CsvData trainData, int startIndex, int endIndex, SVMParams params);
double classificationAccuracy(ClassifiedData set);
void writeAndPrintConfusionMatrix(int** matrix, char** classes, int count, char* type);
void createConfusionMatrix(ClassifiedData set, char* type);

ProgramParams readProgramParams()
{
	ProgramParams params;
	char line[STRINGBUFF*2], param[STRINGBUFF], value[STRINGBUFF];
	FILE *fp = fopen("params.txt", "r");

	if (NULL == fp)
	{
		params.error = 1;
		return params;
	}
	else 
	{
		params.error = 0;
	}

	params.svmParams = DefaultParams();

	while (fgets(line, STRINGBUFF*2-1, fp)) 
	{
		sscanf(line, "%s : %s", param, value);
		
		if (strcmp(param, "numOfFiles") == 0)
		{
			params.fileNumber = atoi(value);
		}
		if (strcmp(param, "firstFile") == 0)
		{
			strcpy(params.firstFile, value);
		}
		if (strcmp(param, "secondFile") == 0)
		{
			strcpy(params.secondFile, value);
		}
		if (strcmp(param, "proportion") == 0)
		{
			params.proportion = atoi(value);
		}
		if (strcmp(param, "normalize") == 0)
		{
			params.normalize = atoi(value);
		}
		if (strcmp(param, "crossValid") == 0)
		{
			strcpy(params.crossValid, value);
		}
		if (strcmp(param, "repet") == 0)
		{
			params.repet = atoi(value);
		}
		if (strcmp(param, "c") == 0)
		{
			params.svmParams.c = atof(value);
		}
		if (strcmp(param, "kernel") == 0)
		{
			params.svmParams.kernel = atoi(value);
		}
		if (strcmp(param, "tol") == 0)
		{
			params.svmParams.tol = atof(value);
		}
		if (strcmp(param, "gamma") == 0)
		{
			params.svmParams.gamma = atof(value);
		}
		if (strcmp(param, "c0") == 0)
		{
			params.svmParams.c0 = atof(value);
		}
		if (strcmp(param, "deg") == 0)
		{
			params.svmParams.deg = atoi(value);
		}
	}

	fclose(fp);
	return params;
}

CsvData allocCsvData(int rows, int columns)
{
	CsvData csvData;
	int i;

	csvData.columns = columns;
	csvData.rows = rows;

	csvData.headers = (char**)malloc(sizeof(char*) * (columns + 1));
	for (i = 0; i < (columns + 1); i++)
	{
		csvData.headers[i] = (char *)malloc(sizeof(char) * STRINGBUFF);
	}

	csvData.classes = (char**)malloc(sizeof(char*) * (rows));
	for (i = 0; i < rows; i++)
	{
		csvData.classes[i] = (char *)malloc(sizeof(char) * STRINGBUFF);
	}

	csvData.data = (double**)malloc(sizeof(double*) * (rows));
	for (i = 0; i < rows; i++)
	{
		csvData.data[i] = (double *)malloc(sizeof(double) * (columns));
	}
	return csvData;
}

void freeCsvData( CsvData csvData)
{
	int i;
	int columns = csvData.columns;
	int rows = csvData.rows;

	for (i = 0; i < (columns + 1); i++)
	{
		free(csvData.headers[i]);
	}
	free(csvData.headers);

	for (i = 0; i < rows; i++)
	{
		free(csvData.classes[i]);
	}
	free(csvData.classes);

	for (i = 0; i < rows; i++)
	{
		free(csvData.data[i]);
	}
	free(csvData.data);
}

void shuffleCsvData(CsvData* data)
{
	int loopCount = data->rows;
	int columns = data->columns;
	int i, j, first, second;
	srand(time(NULL));

	char tempClass[STRINGBUFF];
	double tempValue;

	for (i = 0; i < 2* loopCount; i++)
	{
		first = rand() % loopCount;
		second = rand() % loopCount;

		for (j = 0; j < columns; j++)
		{
			tempValue = data->data[first][j];
			data->data[first][j] = data->data[second][j];
			data->data[second][j] = tempValue;
		}

		strcpy(tempClass, data->classes[first]);
		strcpy(data->classes[first], data->classes[second]);
		strcpy(data->classes[second], tempClass);
	}
}

int readFile(char* fileName,  CsvData* csvData)
{
	int rows, columns, crow, cclass;
	int i, j, k;
	char c;
	char preValue[20];
	double value;
	char line[STRINGBUFF*20];
	FILE* stream = fopen(fileName, "r");

	if (NULL == stream)
	{
		printf("Podano zly plik");
		return 0;
	}

	rows = 0;
	columns = 1;
	while ((c = fgetc(stream)) != EOF) {

		if (c == ',' && 0 == rows) {
			columns++;
		}

		if (c == '\n') {
			rows++;
		}
	}
	fseek(stream, 0, SEEK_SET);

	*csvData = allocCsvData(rows - 1, columns - 1);
	crow = 0;
	while (fgets(line, STRINGBUFF*20-1, stream))
	{
		if (0 == crow) // headery
		{
			i = 0;
			k = 0;
			j = 0;
			while (line[k] != '\n')
			{
				if (line[k] == ',') {
					csvData->headers[i][j] = '\0';
					i++;
					j = 0;
					k++;
					continue;
				}
				csvData->headers[i][j] = line[k];

				j++;
				k++;
			}
			csvData->headers[i][j] = '\0';
		} // headery
		else { // reszta

			i = 0;
			k = 0;
			j = 0;
			cclass = 1;

			while (line[k] != '\n')
			{
				if (line[k] == ',')
				{
					if (1 == cclass)
					{
						csvData->classes[crow - 1][j] = '\0';
						cclass = 0;
						j = 0;
					}
					else
					{
						preValue[j] = '\0';
						j = 0;
						value = atof(preValue);
						csvData->data[crow - 1][i] = value;
						i++;
					}

					k++;
					continue;
				}

				if (1 == cclass)
				{
					csvData->classes[crow - 1][j] = line[k];
				}
				else
				{
					preValue[j] = line[k];
				}
				k++;
				j++;
			}
			preValue[j] = '\0';
			value = atof(preValue);
			csvData->data[crow - 1][i] = value;

		} // reszta
		crow++;
	}

	if (NULL != stream)
	{
		fclose(stream);
	}

	shuffleCsvData(csvData);

	return 1;
}

void readOneFile(CsvData* trainData,  CsvData* testData, char* fileName, int prop)
{
	CsvData oneFileData;
	int trainRows, testRows;
	int i, j, k;
	readFile(fileName, &oneFileData);

	trainRows = (oneFileData.rows * prop) / 100;
	testRows = oneFileData.rows - trainRows;

	*trainData = allocCsvData(trainRows, oneFileData.columns);
	*testData = allocCsvData(testRows, oneFileData.columns);

	for (i = 0; i < oneFileData.columns + 1; i++)
	{
		strcpy(trainData->headers[i], oneFileData.headers[i]);
		strcpy(testData->headers[i], oneFileData.headers[i]);
	}

	for (i = 0; i < trainRows; i++)
	{
		strcpy(trainData->classes[i], oneFileData.classes[i]);


		for (j = 0; j < oneFileData.columns; j++)
		{
			trainData->data[i][j] = oneFileData.data[i][j];
		}
	}

	k = 0;
	for (i = trainRows; i < oneFileData.rows; i++)
	{
		strcpy(testData->classes[k], oneFileData.classes[i]);

		for (j = 0; j < oneFileData.columns; j++)
		{
			testData->data[k][j] = oneFileData.data[i][j];
		}
		k++;
	}

	freeCsvData(oneFileData);
}

void normalizeData(CsvData* data)
{
	double* min = (double *)malloc(sizeof(double) * (data->columns));
	double* max = (double *)malloc(sizeof(double) * (data->columns));
	int i, j;
	double value;

	for (j = 0; j < data->columns; j++)
	{
		max[j] = DBL_MIN;
		min[j] = DBL_MAX;
	}

	for (i = 0; i < data->rows; i++)
	{
		for (j = 0; j < data->columns; j++)
		{
			value = data->data[i][j];
			max[j] = value >= max[j] ? value : max[j];
			min[j] = value <= min[j] ? value : min[j];
		}
	}

	for (i = 0; i < data->rows; i++)
	{
		for (j = 0; j < data->columns; j++)
		{
			data->data[i][j] = (data->data[i][j] - min[j]) / (max[j] - min[j]);
		}
	}

	free(min);
	free(max);
}

int typeOfCrossValidation(char* input, int rows)
{
	int cv, val;
	if (strcmp(input, "LOO") == 0)
	{
		cv = rows;
	}
	else
	{
		val = atoi(input);
		if (val >= 1 && val <= 10)
		{
			cv = val;
		}
	}

	return cv;
}

void crossValidateFold(CsvData trainData, int startIndex, int endIndex, SVMParams params)
{
	int rowsCount = trainData.rows - (endIndex - startIndex);
	CsvData trainSet, testSet;
	trainSet = allocCsvData(rowsCount, trainData.columns);
	testSet = allocCsvData(trainData.rows - rowsCount, trainData.columns);
	int i, j, k, l;

	for (i = 0; i < trainData.columns + 1; i++)
	{
		strcpy(trainSet.headers[i], trainData.headers[i]);
		strcpy(testSet.headers[i], trainData.headers[i]);
	}

	k = 0;
	l = 0;
	for (i = 0; i < trainData.rows; i++)
	{
		if (i >= startIndex && i < endIndex) 
		{
			strcpy(testSet.classes[k], trainData.classes[i]);
			for (j = 0; j < trainData.columns; j++)
			{
				testSet.data[k][j] = trainData.data[i][j];
			}
			k++;
		}
		else
		{
			strcpy(trainSet.classes[l], trainData.classes[i]);
			for (j = 0; j < trainData.columns; j++)
			{
				trainSet.data[l][j] = trainData.data[i][j];
			}
			l++;
		}
	}

	ClassificationResult res = classify(trainSet, testSet, params);
	createConfusionMatrix(res.testSet, "crossValidTest");
	createConfusionMatrix(res.trainSet, "crossValidTrain");

	freeCsvData(trainSet);
	freeCsvData(testSet);
}

void crossValidate(CsvData trainData, int cvk, SVMParams params)
{
	if (cvk == 1)
	{
		return;
	}

	int startIndex, endIndex, newEndIndex;
	int i;
	int testSetAmount = trainData.rows / cvk;

	startIndex = 0;
	endIndex = testSetAmount;
	for (int i = 0; i < cvk; i++)
	{
		crossValidateFold(trainData, startIndex, endIndex, params);
		startIndex = endIndex;
		newEndIndex = endIndex + testSetAmount;
		endIndex = newEndIndex > trainData.rows ? trainData.rows : newEndIndex;
	}
}

double classificationAccuracy(ClassifiedData set)
{
	int correctClassified = 0;
	for (int i = 0; i < set.rows; i++)
	{
		if (strcmp(set.classes[i], set.assignedClasses[i]) == 0)
		{
			correctClassified += 1;
		}
	}
	double	ratio = (double)correctClassified / (double)set.rows;
	return ratio;
}

void writeAndPrintConfusionMatrix(int** matrix, char** classes, int count, char* type)
{
	char fileName[STRINGBUFF];
	strcpy(fileName, type);
	strcat(fileName, "ConfusionMatrix.csv");
	FILE *fp = fopen(fileName, "ab+");
	int i,j;
	if (fp)
	{
		printf("%s confusion matrix \n", type);

		fprintf(fp, "%s", "X");
		printf("%s", "X");
		for (i = 0; i < count; i++)
		{
			fprintf(fp, ",%s", classes[i]);
			printf(",%s", classes[i]);
		}
		fprintf(fp, "\r\n");
		printf("\n");

		for (i = 0; i < count; i++)
		{
			fprintf(fp, "%s", classes[i]);
			printf("%s", classes[i]);
			for (j = 0; j < count; j++)
			{
				fprintf(fp, ",%d", matrix[i][j]);
				printf(",%d", matrix[i][j]);
			}
			fprintf(fp, "\r\n");
			printf("\n");
		}
	}
	fclose(fp);
	printf("\n");
}

void createConfusionMatrix(ClassifiedData set, char* type)
{
	if (set.rows == 0) return;

	int i, j, any, count;
	int actual, predicted;
	count = 1;
	char** classes = (char**)malloc(sizeof(char*));
	classes[count-1] = (char *)malloc(sizeof(char) * STRINGBUFF);
	strcpy(classes[0], set.classes[0]);
	char** tmpClasses;
	int** cmatrix;

	for (i = 1; i < set.rows; i++)
	{
		any = 0;
		for (j = 0; j < count; j++)
		{
			if (strcmp(classes[j], set.classes[i]) == 0) any = 1;
		}

		if (any == 0)
		{
			count = count + 1;
			tmpClasses = (char**)realloc(classes, count * sizeof(char*));
			if (tmpClasses)
			{
				classes = tmpClasses;
				classes[count - 1] = (char *)malloc(sizeof(char) * STRINGBUFF);
				strcpy(classes[count - 1], set.classes[i]);
			}
		}
	}

	cmatrix = (int**)malloc(sizeof(int*) * (count));
	for (i = 0; i < count; i++)
	{
		cmatrix[i] = (int *)malloc(sizeof(int) * (count));
	}

	for (i = 0; i < count; i++)
	{
		for (j = 0; j < count; j++)
		{
			cmatrix[i][j] = 0;
		}
	}
	actual = 0;
	predicted = 0;
	for (i = 0; i < set.rows; i++)
	{
		for (j = 0; j < count; j++)
		{
			if (strcmp(set.classes[i], classes[j]) == 0) actual = j;
			if (strcmp(set.assignedClasses[i], classes[j]) == 0) predicted = j;
		}
		cmatrix[predicted][actual] += 1;
	}

	writeAndPrintConfusionMatrix(cmatrix, classes, count, type);

	for (i = 0; i < count; i++)
	{
		free(cmatrix[i]);
	}
	free(cmatrix);

	for (i = 0; i < count; i++)
	{
		free(classes[i]);
	}
	free(classes);
}

int main()
{
	 CsvData trainData, testData;
	 int i, crossValidType;
	 double testRatioSum, trainRatioSum, testRatio, trainRatio;
	 ClassificationResult res;
	 testRatioSum = 0;
	 trainRatioSum = 0;

	 ProgramParams programParams = readProgramParams();
	 if (programParams.error == 1)
	 {
		 printf("Blad wczytania parametrow programu\n");
		 return EXIT_FAILURE;
	 }

	if (programParams.fileNumber == 1)
	{
		readOneFile(&trainData, &testData, programParams.firstFile, programParams.proportion);
	}
	else
	{
		readFile(programParams.firstFile, &trainData);
		readFile(programParams.secondFile, &testData);
	}

	if (programParams.normalize == 1)
	{
		normalizeData(&trainData);
		normalizeData(&testData);
	}

	crossValidType = typeOfCrossValidation(programParams.crossValid, testData.rows);

	for (i = 0; i < programParams.repet; i++)
	{
		printf("******************************* poczatek iteracji %d ***************************\n", i + 1);

		crossValidate(testData, crossValidType, programParams.svmParams);
		ClassificationResult res = classify(trainData, testData, programParams.svmParams);
		createConfusionMatrix(res.testSet, "test");
		createConfusionMatrix(res.trainSet, "train");

		trainRatio = classificationAccuracy(res.trainSet);
		testRatio = classificationAccuracy(res.testSet);
		printf("Jakosc klasyfikacji zbioru trenujacego: %f\n", trainRatio);
		printf("Jakosc klasyfikacji zbioru testowego: %f\n", testRatio);

		trainRatioSum += trainRatio;
		testRatioSum += testRatio;

		printf("******************************* koniec iteracji %d *****************************\n\n", i + 1);
	}

	printf("Wykonano %d powtorzen eksperymentu \n", programParams.repet);
	printf("Srednia jakosc klasyfikacji zbioru trenujacego: %f\n", trainRatioSum/ programParams.repet);
	printf("Jakosc klasyfikacji zbioru testowego: %f\n", testRatioSum/ programParams.repet);

	freeCsvData(testData);
	freeCsvData(trainData);

	system("pause");
	return 0;
}

