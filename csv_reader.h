#pragma once
#include <stdio.h>
#include "httpd.h"
FILE  *ncsv_openFile(const char *pt_fileName, const char *mode);
void ncsv_closeFile(FILE *pt_file);
const char ncsv_getDelimiter(FILE *pt_file);
int ncsv_getFieldsCount(FILE *pt_file, const char *delimiter);
void ncsv_parseCsvLine(const char *line, const char *delimiter, const int *fieldsCount, char **fields, request_rec *r);

