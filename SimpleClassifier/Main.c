#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getfield(char* line, int num)
{
	const char* tok;
	for (tok = strtok_s(line, 1024, ",");
		tok && *tok;
		tok = strtok_s(NULL, 1024, ",\n"))
	{
		if (!--num)
			return tok;
	}
	return NULL;
}

void readFile(char* fileName) 
{
	FILE* stream;
	fopen_s(&stream ,fileName, "r");

	char line[1024];
	while (fgets(line, 1024, stream))
	{
		char* tmp = strdup_s(line);
		printf("Field 3 would be %s\n", getfield(tmp, 3));
		free(tmp);
	}
}

char* askForString() 
{
	int rc;
	char buff[1025];

	rc = getLine("Enter string> ", buff, sizeof(buff));
	if (-1 != rc)
		puts(buff);

	return buff;
}

int main()
{
	readFile(askForString());
	return 0;
}

