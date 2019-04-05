#include "DataTypes.h"

ClassificationResult classify(struct CsvData trainSet, struct CsvData testSet, SVMParams params);
SVMParams DefaultParams(int dim);
