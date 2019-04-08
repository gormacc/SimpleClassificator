#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "SVM.h"

#define STRINGBUFF 64

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
			strcpy(params.firstFile, value);
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

void printData( CsvData data)
{
	int i, j;
	printf("Headers:\n");
	for (i = 0; i < data.columns + 1; i++)
	{
		printf("%s\n", data.headers[i]);
	}

	printf("Classes:\n");
	for (i = 0; i < data.rows; i++)
	{
		printf("%s\n", data.classes[i]);
	}

	printf("Data:\n");
	for (i = 0; i < data.rows; i++)
	{
		for (j = 0; j < data.columns; j++)
		{
			printf("%f  ", data.data[i][j]);
		}
		printf("\n");
	}
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
	FILE* stream;
	errno_t err = fopen_s(&stream, fileName, "r");

	if (NULL == stream)
	{
		printf("Podano zly plik");
		return 0;
	}

	int rows = 0;
	int columns = 1;
	char c;
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
	char line[STRINGBUFF];
	int crow = 0;
	int i, j, k;

	while (fgets(line, STRINGBUFF, stream))
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
			char preValue[20];
			double value;
			int cclass = 1;

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

	return 1;
}

int askForFilesNumber()
{
	int keepAsking = 1;
	int numOfFiles;
	char answer[10];
	while (keepAsking == 1)
	{
		printf("Podaj ilosc plikow do wczytania\n");
		scanf_s("%s", answer, sizeof(answer));
		numOfFiles = atoi(answer);
		if (numOfFiles == 1 || numOfFiles == 2)
		{
			keepAsking = 0;
		}
		else
		{
			printf("Podano zla ilosc plikow\n");
		}
	}
	return numOfFiles;
}

void readOneFile(CsvData* trainData,  CsvData* testData, char* fileName, int prop)
{
	CsvData oneFileData;
	if (readFile(fileName, &oneFileData) == 1)

	shuffleCsvData(&oneFileData);

	int trainRows = (oneFileData.rows * prop) / 100;
	int testRows = oneFileData.rows - trainRows;

	*trainData = allocCsvData(trainRows, oneFileData.columns);
	*testData = allocCsvData(testRows, oneFileData.columns);

	int i, j, k;

	for (i = 0; i < oneFileData.columns + 1; i++)
	{
		/*j = 0;
		char c = oneFileData.headers[i][j];
		while (c != '\0')
		{
			trainData->headers[i][j] = c;
			testData->headers[i][j] = c;
			j++;
			c = oneFileData.headers[i][j];
		}
		trainData->headers[i][j] = '\0';
		testData->headers[i][j] = '\0';*/

		strcpy(trainData->headers[i], oneFileData.headers[i]);
		strcpy(testData->headers[i], oneFileData.headers[i]);
	}

	for (i = 0; i < trainRows; i++)
	{
		j = 0;
		char c = oneFileData.classes[i][j];
		while (c != '\0')
		{
			trainData->classes[i][j] = c;
			j++;
			c = oneFileData.classes[i][j];
		}
		trainData->classes[i][j] = '\0';

		for (j = 0; j < oneFileData.columns; j++)
		{
			trainData->data[i][j] = oneFileData.data[i][j];
		}
	}

	k = 0;
	for (i = trainRows; i < oneFileData.rows; i++)
	{
		j = 0;
		char c = oneFileData.classes[i][j];
		while (c != '\0')
		{
			testData->classes[k][j] = c;
			j++;
			c = oneFileData.classes[i][j];
		}
		testData->classes[k][j] = '\0';

		for (j = 0; j < oneFileData.columns; j++)
		{
			testData->data[k][j] = oneFileData.data[i][j];
		}
		k++;
	}

	freeCsvData(oneFileData);
}

void readTwoFiles( CsvData* trainData,  CsvData* testData)
{
	char fileName[STRINGBUFF];
	int keepAsking = 1;
	while (keepAsking == 1)
	{
		printf("Podaj nazwe pierwszego pliku\n");
		scanf_s("%s", fileName, sizeof(fileName));
		if (readFile(fileName, trainData) == 1)
		{
			keepAsking = 0;
		}
	}

	keepAsking = 1;
	while (keepAsking == 1)
	{
		printf("Podaj nazwe drugiego pliku\n");
		scanf_s("%s", fileName, sizeof(fileName));
		if (readFile(fileName, testData) == 1)
		{
			keepAsking = 0;
		}
	}

}

void normalizeData( CsvData* data)
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

int askForNormalization()
{
	int keepAsking = 1;
	char answer[10];
	int normalize;
	while (keepAsking == 1)
	{
		printf("Czy chcesz znormalizowac dane?\n");
		scanf_s("%s", answer, sizeof(answer));

		if (strcmp(answer, "tak") == 0)
		{
			normalize = 1;
			keepAsking = 0;
		}
		else if (strcmp(answer, "nie") == 0)
		{
			normalize = 0;
			keepAsking = 0;
		}
		else
		{
			printf("Nieoczekiwana odpowiedz\n");
		}
	}
	return normalize;
}

int askForCrossValidation(int rows)
{
	int keepAsking = 1;
	char answer[10];
	int cv, val;
	while (keepAsking == 1)
	{
		printf("Podaj typ walidacji\n");
		scanf_s("%s", answer, sizeof(answer));

		if (strcmp(answer, "LOO") == 0)
		{
			cv = rows;
			keepAsking = 0;
		}
		else
		{
			val = atoi(answer);
			if (val >= 1 && val <= 10)
			{
				cv = val;
				keepAsking = 0;
			}
		}
	}
	return cv;
}

 CsvData deleteRowsInSet( CsvData trainData, int startIndex, int endIndex,  CsvData* newSet)
{
	int rowsCount = trainData.rows - (endIndex - startIndex);
	*newSet = allocCsvData(rowsCount, trainData.columns);
	int i, j, k;

	for (i = 0; i < trainData.columns + 1; i++)
	{
		j = 0;
		char c = trainData.headers[i][j];
		while (c != '\0')
		{
			newSet->headers[i][j] = c;
			j++;
			c = trainData.headers[i][j];
		}
		newSet->headers[i][j] = '\0';
	}

	k = 0;
	for (i = 0; i < trainData.rows; i++)
	{
		if (i >= startIndex && i < endIndex) continue;
		j = 0;
		char c = trainData.classes[i][j];
		while (c != '\0')
		{
			newSet->classes[k][j] = c;
			j++;
			c = trainData.classes[i][j];
		}
		newSet->classes[k][j] = '\0';

		for (j = 0; j < trainData.columns; j++)
		{
			newSet->data[k][j] = trainData.data[i][j];
		}
		k++;
	}
}

void crossValidate( CsvData trainData, int cvk)
{
	if (cvk == 1)
	{
		//run the SVM on one set and return
	}

	int trainSetAmount = trainData.rows / cvk;
	int startIndex, endIndex, newEndIndex;
	int i;
	 CsvData tempTrainSet;

	startIndex = 0;
	for (int i = 0; i < cvk; i++)
	{
		newEndIndex = startIndex + trainSetAmount;
		endIndex = newEndIndex > trainData.rows ? trainData.rows : newEndIndex;
		deleteRowsInSet(trainData, startIndex, endIndex, &tempTrainSet);

		printf("\n\n %d z %d zbiorow testowych z cross walidacji : \n\n", i + 1, cvk);
		printData(tempTrainSet);

		// run the SVM on the created set
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
		printf("\n%s confusion matrix \n", type);

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
	printf("\n\n");
}

void createConfusionMatrix(ClassifiedData set, char* type)
{
	if (set.rows == 0) return;

	int i, j, any, count;
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

	/*printf("Klasy\n\n\n\n");

	for (i = 0; i < count; i++)
	{
		printf("%s\n", classes[i]);
	}*/

	cmatrix = (int**)malloc(sizeof(int*) * (count));
	for (i = 0; i < count; i++)
	{
		cmatrix[i] = (int *)malloc(sizeof(int) * (count));
	}

	int actual, predicted;

	for (i = 0; i < count; i++)
	{
		for (j = 0; j < count; j++)
		{
			cmatrix[i][j] = 0;
		}
	}

	for (i = 0; i < set.rows; i++)
	{
		for (j = 0; j < count; j++)
		{
			if (strcmp(set.classes[i], classes[j]) == 0) actual = j;
			if (strcmp(set.assignedClasses[i], classes[j]) == 0) predicted = j;
		}
		cmatrix[predicted][actual] += 1;
	}

	/*printf("Matrix\n\n\n\n");

	for (i = 0; i < count; i++)
	{
		for (j = 0; j < count; j++)
		{
			printf(" %d ", cmatrix[i][j]);
		}
		printf("\n");
	}*/

	writeAndPrintConfusionMatrix(cmatrix, classes, count, type);
}



int main()
{
	 CsvData trainData, testData;
	 int i;
	 double testRatioSum, trainRatioSum;
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
		readTwoFiles(&trainData, &testData);
	}

/*	printf("\n\nWczytano podany zbior trenujacy : \n\n");
	printData(trainData);

	printf("\n\n\n\n");
	shuffleCsvData(&trainData);
	shuffleCsvData(&testData);*/

	
/*	printf("\n\nWczytano podany zbior testowy : \n\n");
	printData(testData);*/

/*	SVMParams params = DefaultParams();
	params.c = 5;
	params.c0 = 0;
	params.kernel = rbf;
	params.deg = 2;
	params.gamma = 10;
	params.tol = 0.00001;*/

	for (i = 0; i < programParams.repet; i++)
	{
		ClassificationResult res = classify(trainData, testData, programParams.svmParams);
		createConfusionMatrix(res.testSet, "test");
		createConfusionMatrix(res.trainSet, "train");

		double trainRatio = classificationAccuracy(res.trainSet);
		double testRatio = classificationAccuracy(res.testSet);
		printf("Jakosc klasyfikacji zbioru trenujacego: %f\n", trainRatio);
		printf("Jakosc klasyfikacji zbioru testowego: %f\n", testRatio);

		trainRatioSum += trainRatio;
		testRatioSum += testRatio;
	}

	printf("Wykonano %d powtorzen eksperymentu \n", programParams.repet);
	printf("Srednia jakosc klasyfikacji zbioru trenujacego: %f\n", trainRatioSum/ programParams.repet);
	printf("Jakosc klasyfikacji zbioru testowego: %f\n", testRatioSum/ programParams.repet);

	

	/*
	if (askForNormalization() == 1)
	{
		normalizeData(&trainData);
		normalizeData(&testData);

		printf("\n\n Zbior trenujacy po normalizacji : \n\n");
		printData(trainData);
		printf("\n\n Zbior testowy po normalizacji : \n\n");
		printData(testData);
	}
	
	crossValidate(trainData, askForCrossValidation(trainData.rows));

	*/

	freeCsvData(testData);
	freeCsvData(trainData);

	system("pause");
	return 0;
	//return 0;
}

