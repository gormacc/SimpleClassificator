typedef struct {
	int columns;
	int rows;
	char** headers;
	char** classes;
	double** data;
}CsvData;

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
	rbf,
	poly,
	sinc,
	multiquadratic,
	cauchy,
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
	double* alfa;
	int* y;
	double b;
	double** x;
	int vectorsCount;// równe liczbie CsvData.rows;
	int dim;// równe liczbie CsvData.columns;
}BinaryClassificator;

