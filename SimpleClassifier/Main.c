#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CsvData {
	int columns;
	int rows;
	char** headers;
	char** classes;
	double** data;
};

void readFile(char* fileName) 
{
	FILE* stream;
	errno_t err = fopen_s(&stream , fileName, "r");
	struct CsvData csvData;

	if (NULL == stream) 
	{
		printf("Podano zly plik");
		return EXIT_FAILURE;
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

	csvData.columns = columns - 1;
	csvData.rows = rows - 1;


	char line[1024];
	int crow = 0;
	int i,j,k;

	csvData.headers = (char**)malloc(sizeof(char*) * columns);
	for (i = 0; i < columns; i++)
	{
		csvData.headers[i] = (char *)malloc(sizeof(char) * 1024);
	}

	csvData.classes = (char**)malloc(sizeof(char*) * (rows - 1));
	for (i = 0; i < rows-1; i++)
	{
		csvData.classes[i] = (char *)malloc(sizeof(char) * 1024);
	}

	csvData.data = (double**)malloc(sizeof(double*) * (rows - 1));
	for (i = 0; i < rows - 1; i++)
	{
		csvData.data[i] = (double *)malloc(sizeof(double) * (columns - 1));
	}
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
					csvData.headers[i][j] = '\0';
					i++;
					j = 0;
					k++;
					continue;
				}
				csvData.headers[i][j] = line[k];

				j++;
				k++;
			}
			csvData.headers[i][j] = '\0';
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
						csvData.classes[crow-1][j] = '\0';
						cclass = 0;
						j = 0;
					}
					else 
					{
						preValue[j] = '\0';
						j = 0;
						value = atof(preValue);
						csvData.data[crow - 1][i] = value;
						i++;
					}

					k++;
					continue;
				}

				if (1 == cclass) 
				{
					csvData.classes[crow-1][j] = line[k];
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
			csvData.data[crow - 1][i] = value;

		} // reszta
		crow++;
	}

	if (NULL != stream) 
	{
		fclose(stream);
	}

	// sprawdzenie

	printf("Headers:\n");
	for (i = 0; i < columns; i++) 
	{
		printf("%s\n", csvData.headers[i]);
	}

	printf("Classes:\n");
	for (i = 0; i < rows-1; i++)
	{
		printf("%s\n", csvData.classes[i]);
	}

	printf("Data:\n");
	for (i = 0; i < rows - 1; i++)
	{
		for (j = 0; j < columns - 1; j++)
		{
			printf("%f  ", csvData.data[i][j]);
		}
		printf("\n");
	}

	//sprawdzenie

}

int main()
{
	printf("Podaj nazwe pliku\n");
	char fileName[1024];
	scanf_s("%s", fileName, sizeof(fileName));
	
	readFile("iris.csv"); // todo
	scanf_s("%s", fileName, sizeof(fileName));
	return 0;
}

