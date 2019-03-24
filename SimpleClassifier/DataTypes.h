struct CsvData {
	int columns;
	int rows;
	char** headers;
	char** classes;
	double** data;
};

typedef struct {
	int columns;
	int rows;
	char** headers;
	double** data;
	char** classes;
	char** assignedClasses;
}ClassifiedData;

typedef struct {
	ClassifiedData trainSet;
	ClassifiedData testSet;
}ClassificationResult;

typedef enum {
	lin,
	poly,
	rbf
}Kernel;

typedef struct {
	double c;//penalty parameter
	Kernel kernel;
	double tol;//tolerance, stop condition
	double gamma;
	double c0;//coefficient 0 of function , constant number
	int deg;//polynomial degree
}SVMParams;

typedef struct {
	double* w;//penalty parameter
	double b;
}Classificator;

