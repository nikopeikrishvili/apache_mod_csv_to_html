#include "csv_reader.h"
#include "strings.h"
#include "http_protocol.h"
#include "apr_strings.h"
#include "stdbool.h"
#define C2H_CSV_MAX_LINE_LENGTH 2048

FILE  *openFile(const char *pt_fileName, const char *mode)
{
  if(NULL != pt_fileName) {
  	return fopen(pt_fileName, mode);
  }
  return NULL;
}
void closeFile(FILE *pt_file){
  if(NULL != pt_file){
    fclose(pt_file);
  }
}
const char getDelimiter(FILE *pt_file){
  char delimiters[] = {',', ';'};
  int counts[sizeof(delimiters)] = {0};
  char line[C2H_CSV_MAX_LINE_LENGTH];
  rewind(pt_file);
  // Read the first line of the file
  if (fgets(line, sizeof(line), pt_file) != NULL) {
	size_t lineSize = strlen(line);
	size_t delimitersSize = sizeof(delimiters);
    for (size_t i = 0; i < lineSize; i++) {
      for (size_t j = 0; j < delimitersSize; j++) {
        if (line[i] == delimiters[j]) {
          counts[j]++;
        }
      }
    }
  }

  // Find the delimiter with the highest count
  int max_count = 0;
  char detected_delimiter = ',';
  size_t delimitersSize = sizeof(delimiters);
  for (size_t i = 0; i < delimitersSize; i++) {
    if (counts[i] > max_count) {
      max_count = counts[i];
      detected_delimiter = delimiters[i];
    }
  }

  return detected_delimiter;
}
/**
* Get fileds count for current CSV file by taking first row
*
*
*/
 int getFieldsCount(FILE *pt_file, const char *delimiter){
  char line[C2H_CSV_MAX_LINE_LENGTH];
  int count = {0};
  rewind(pt_file);
  // Read the first line
  if (fgets(line, sizeof(line), pt_file) == NULL) {
    return count;
  }
  count = 1;
  size_t lineSize = strlen(line);
  for (size_t i = 0; i < lineSize; i++) {
    if (line[i] == *delimiter) {
      count++;
    }
  }

  return count;
}
/**
 * Parses a single line of CSV data into an array of fields.
 *
 * @param line The CSV line to parse.
 * @param delimiter - delimiter that is used in current CSV file
 * @oaram fieldsCount - how many fields are in this file per row
 * @param fields Array to store pointers to parsed fields.
 * @param r Apache request struct
 * @return The number of fields parsed.
 */
int parseCsvLine(const char *line, const char *delimiter, const int *fieldsCount, char **fields, request_rec *r) {
  const char *ptr = line;
  int field_count = 0;
  bool in_quotes = false;
  char buffer[*fieldsCount];
  int buffer_pos = 0;

  while (*ptr != '\0') {
    if (in_quotes) {
      if (*ptr == '"') {
        // Handle double quotes within quoted field
        if (*(ptr + 1) == '"') {
          buffer[buffer_pos++] = '"';
          ptr++; // Skip the second quote
        } else {
          in_quotes = false; // End of quoted field
        }
      } else {
        buffer[buffer_pos++] = *ptr;
      }
    } else {
      if (*ptr == *delimiter) {
        // End of field
        buffer[buffer_pos] = '\0';
        fields[field_count++] = apr_pstrdup(r->pool, buffer);
        buffer_pos = 0;
      } else if (*ptr == '"') {
        in_quotes = true; // Start of quoted field
      } else {
        buffer[buffer_pos++] = *ptr;
      }
    }
    ptr++;
  }
  // Add the last field
  buffer[buffer_pos] = '\0';
  fields[field_count++] = apr_pstrdup(r->pool,buffer);

  return field_count;
}