#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "SVM.h"



void printData(struct CsvData data)
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

struct CsvData allocCsvData(int rows, int columns)
{
	struct CsvData csvData;
	int i;

	csvData.columns = columns;
	csvData.rows = rows;

	csvData.headers = (char**)malloc(sizeof(char*) * (columns + 1));
	for (i = 0; i < (columns + 1); i++)
	{
		csvData.headers[i] = (char *)malloc(sizeof(char) * 1024);
	}

	csvData.classes = (char**)malloc(sizeof(char*) * (rows));
	for (i = 0; i < rows; i++)
	{
		csvData.classes[i] = (char *)malloc(sizeof(char) * 1024);
	}

	csvData.data = (double**)malloc(sizeof(double*) * (rows));
	for (i = 0; i < rows; i++)
	{
		csvData.data[i] = (double *)malloc(sizeof(double) * (columns));
	}
	return csvData;
}

int readFile(char* fileName, struct CsvData* csvData)
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
	char line[1024];
	int crow = 0;
	int i, j, k;

	while (fgets(line, 1024, stream))
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

void readOneFile(struct CsvData* trainData, struct CsvData* testData)
{
	char fileName[1024];
	int keepAsking = 1;
	struct CsvData oneFileData;
	while (keepAsking == 1)
	{
		printf("Podaj nazwe pliku\n");
		scanf_s("%s", fileName, sizeof(fileName));
		if (readFile(fileName, &oneFileData) == 1)
		{
			keepAsking = 0;
		}
	}

	//printf("Wczytano poni¿sze dane\n\n");
	//printData(oneFileData);

	char proportion[10];
	keepAsking = 1;
	int prop;
	while (keepAsking == 1)
	{
		printf("\n\nPodaj proporcje podzialu na dane trenujace i testowe\n");
		scanf_s("%s", proportion, sizeof(proportion));
		prop = atoi(proportion);
		if (prop > 0 && prop < 100)
		{
			keepAsking = 0;
		}
		else
		{
			printf("Podano zle proporcje\n");
		}
	}

	int trainRows = (oneFileData.rows * prop) / 100;
	int testRows = oneFileData.rows - trainRows;

	*trainData = allocCsvData(trainRows, oneFileData.columns);
	*testData = allocCsvData(testRows, oneFileData.columns);

	int i, j, k;

	for (i = 0; i < oneFileData.columns + 1; i++)
	{
		j = 0;
		char c = oneFileData.headers[i][j];
		while (c != '\0')
		{
			trainData->headers[i][j] = c;
			testData->headers[i][j] = c;
			j++;
			c = oneFileData.headers[i][j];
		}
		trainData->headers[i][j] = '\0';
		testData->headers[i][j] = '\0';
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
}

void readTwoFiles(struct CsvData* trainData, struct CsvData* testData)
{
	char fileName[1024];
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

void normalizeData(struct CsvData* data)
{
	double min = DBL_MAX;
	double max = DBL_MIN;
	int i, j;
	double value;

	for (i = 0; i < data->rows; i++)
	{
		for (j = 0; j < data->columns; j++)
		{
			value = data->data[i][j];
			max = value >= max ? value : max;
			min = value <= min ? value : min;
		}
	}

	for (i = 0; i < data->rows; i++)
	{
		for (j = 0; j < data->columns; j++)
		{
			data->data[i][j] = (data->data[i][j] - min) / (max - min);
		}
	}
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

struct CsvData deleteRowsInSet(struct CsvData trainData, int startIndex, int endIndex, struct CsvData* newSet)
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

void crossValidate(struct CsvData trainData, int cvk)
{
	if (cvk == 1)
	{
		//run the SVM on one set and return
	}

	int trainSetAmount = trainData.rows / cvk;
	int startIndex, endIndex, newEndIndex;
	int i;
	struct CsvData tempTrainSet;

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

int main()
{
	struct CsvData trainData;
	struct CsvData testData;
	if (askForFilesNumber() == 1)
	{
		readOneFile(&trainData, &testData);
	}
	else
	{
		readTwoFiles(&trainData, &testData);
	}

	printf("\n\nWczytano podany zbior trenujacy : \n\n");
	printData(trainData);
	printf("\n\nWczytano podany zbior testowy : \n\n");
	printData(testData);
	SVMParams params = DefaultParams(trainData.columns);
	params.c = 5;
	params.c0 = 0;
	params.kernel = rbf;
	params.deg = 2;
	params.gamma = 10;
	params.tol = 0.00001;

	ClassificationResult res = classify(trainData, testData, params);

	double trainRatio = classificationAccuracy(res.trainSet);
	double testRatio = classificationAccuracy(res.testSet);
	printf("Jakosc klasyfikacji zbioru trenujacego: %f\n", trainRatio);
	printf("Jakosc klasyfikacji zbioru testowego: %f\n", testRatio);

	/*if (askForNormalization() == 1)
	{
		normalizeData(&trainData);
		normalizeData(&testData);

		printf("\n\n Zbior trenujacy po normalizacji : \n\n");
		printData(trainData);
		printf("\n\n Zbior testowy po normalizacji : \n\n");
		printData(testData);
	}

	crossValidate(trainData, askForCrossValidation(trainData.rows));

	char fileName[10];
	printf("Podaj proporcje podzialu danych\n");
	scanf_s("%s", fileName, sizeof(fileName));*/
	system("pause");
	return 0;
	//return 0;
}

