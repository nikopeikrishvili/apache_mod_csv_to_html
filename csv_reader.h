#pragma once
#include <stdio.h>
#include "httpd.h"
FILE  *openFile(const char *pt_fileName, const char *mode);
void closeFile(FILE *pt_file);
const char getDelimiter(FILE *pt_file);
int getFieldsCount(FILE *pt_file, const char *delimiter);
void parseCsvLine(const char *line, const char *delimiter, const int *fieldsCount, char **fields, request_rec *r);

