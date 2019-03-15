#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readFile(char* fileName) 
{
	FILE* stream;
	errno_t err = fopen_s(&stream , fileName, "r");

	if (NULL == stream) 
	{
		printf("Podano zly plik");
		return EXIT_FAILURE;
	}

	char line[1024];
	while (fgets(line, 1024, stream))
	{
		printf("%s", line);
	}
	
}

int main()
{
	printf("Podaj nazwe pliku\n");
	char fileName[1024];
	scanf_s("%s", fileName, sizeof(fileName));
	
	readFile(fileName);
	scanf_s("%s", fileName, sizeof(fileName));
	return 0;
}

