#pragma once
/**
*
*/
const char *getFileName(const char *pt_filePath);
void addStyles(request_rec *r);
void renderTable(FILE *pt_file, request_rec *r);
void getFileInfoHeader(const char *pt_fileLocation, request_rec *r);
char *getArgValues(const char *pt_args, const char *pt_field, const request_rec *r);