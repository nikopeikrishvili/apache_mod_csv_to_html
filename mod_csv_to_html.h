#pragma once
/**
*
*/
static const char *getFileName(const char *pt_filePath);
static void addStyles(request_rec *r);
static void renderTable(FILE *pt_file, request_rec *r);
static void getFileInfoHeader(const char *pt_fileLocation, request_rec *r);