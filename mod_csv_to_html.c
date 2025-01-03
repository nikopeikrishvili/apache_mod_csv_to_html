#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <ap_config.h>
#include <apr_strings.h>
#include <apr_tables.h>
#include <util_script.h>
#include "mod_csv_to_html.h"
#include "csv_reader.h"
#include "http_log.h"
/**
 * Main handler for module
 * @param r - Apache request structure
 * @return OK or DECLINED , in case of DECLINED file will be downloaded
 */
static int csv_to_html_handler(request_rec *r) {
  if (strcmp(r->handler, "csv_to_html") != 0) {
    return DECLINED;
  }

  // Download file if we have download argument that is equal to 1
  apr_table_t*GET;           // Create a var table
  ap_args_to_table(r, &GET); // Read data from get
  const char* has_download_arg = apr_table_get(GET, "download");
  if (has_download_arg != NULL && *has_download_arg == '1') {
    return DECLINED; // Download file
  }
  // end download
  r->content_type = "text/html";
  const char *csv_filename = apr_pstrdup(r->pool, r->canonical_filename);
  if (csv_filename == NULL) {
    return DECLINED; // Download file
  }
  const char *filename = getFileName(csv_filename);
  FILE *pt_file = ncsv_openFile(r->canonical_filename, "r");
  if (NULL == pt_file) {
    return DECLINED; // Download file
  }
  ap_rputs("<!DOCTYPE html><html><head><title>", r);
  ap_rputs(filename, r);
  ap_rputs("</title>", r);
  addStyles(r);
  ap_rputs("</head><body style='padding:10px 20px;'>", r);

  getFileInfoHeader(filename, r);

  renderTable(pt_file, r);
  ap_rputs("</body>", r);
  ap_rputs("</html>", r);
  ncsv_closeFile(pt_file);

  return OK;
}

static void csv_to_html_register_hooks(apr_pool_t *p) {
  ap_hook_handler(csv_to_html_handler, NULL, NULL, APR_HOOK_LAST);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA csv_to_html_module = {
    STANDARD20_MODULE_STUFF,
    NULL,                      /* create per-dir    config structures */
    NULL,                      /* merge  per-dir    config structures */
    NULL,                      /* create per-server config structures */
    NULL,                      /* merge  per-server config structures */
    NULL,                      /* table of config file commands       */
    csv_to_html_register_hooks /* register hooks                      */
};

/**
 * Given file path it returns just filename and extension
 * @param pt_filePath - file full pack
 * @return
 */
static const char *getFileName(const char *pt_filePath) {
  // Find the last slash
  const char *last_slash = strrchr(pt_filePath, '/');
  if (last_slash) {
    return last_slash + 1; // Skip the slash
  }
  return pt_filePath;
}

/**
 * Adds styles to html content in <style> tag
 * @param r
 */
static void addStyles(request_rec *r) {
  ap_rputs("<style>\n", r);
  ap_rputs("table {width:100%;border-collapse: collapse;margin: 25px "
           "0;font-size: 0.9em;font-family: sans-serif;box-shadow: 0 0 20px "
           "rgba(0, 0, 0, 0.15);}\n",
           r);
  ap_rputs("table thead tr {background-color: #009879;color: #ffffff; "
           "text-align: left;}\n",
           r);
  ap_rputs("th, td {padding: 12px 15px;}\n", r);
  ap_rputs("table tbody tr{border-bottom: 1px solid #dddddd;}\n", r);
  ap_rputs("table tbody tr:nth-of-type(even){background-color: #f3f3f3;}\n", r);
  ap_rputs(
      "table tbody tbody tr:last-of-type{border-bottom: 2px solid #009879;}\n",
      r);
  ap_rputs("</style>", r);
}

/**
 *
 * @param pt_file - opened file pointer - file descriptor is not NULL it's
 * checked in parent function
 * @param r - Apache request structure
 */
static void renderTable(FILE *pt_file, request_rec *r) {
  ap_rputs("<table>", r);
  ap_rputs("<tbody>", r);
  const char delimiter = ncsv_getDelimiter(pt_file);

  const int fieldsCount = ncsv_getFieldsCount(pt_file, &delimiter);
  char line[LINE_MAX];
  rewind(pt_file);
  while (fgets(line, LINE_MAX, pt_file)) {
    ap_rputs("<tr>", r);
    const char *csv_line = apr_pstrdup(r->pool, line);
    char *fields[fieldsCount];
    ncsv_parseCsvLine(csv_line, &delimiter, &fieldsCount, fields, r);
    for (int i = 0; i < fieldsCount; i++) {
      ap_rputs("<td>", r);
      ap_rputs(ap_escape_html(r->pool, fields[i]), r);
      ap_rputs("</td>", r);
    }
    ap_rputs("</tr>", r);
  }
  ap_rputs("</tbody>", r);
  ap_rputs("</table>", r);
}

/**
 * Generates header block of HTML page, with download button
 * @param pt_fileLocation - file name
 * @param r
 */
static void getFileInfoHeader(const char *pt_fileLocation, request_rec *r) {
  ap_rputs("<div style='align-items: center; display:flex; flex-direction:row; "
           "justify-content: space-between;'>",
           r);
  ap_rputs("<div>", r);
  ap_rputs("<h2>", r);
  ap_rputs(pt_fileLocation, r);
  ap_rputs("</h2>", r);
  ap_rputs("</div>", r);

  ap_rputs("<div style='cursor:pointer;'>", r);
  ap_rputs("<a style='border-radius:5px; text-decoration:none; font-family: "
           "sans-serif; background-color: DodgerBlue; border: none; color: "
           "white; padding: 12px 30px; cursor: pointer; font-size: 20px;' "
           "href='?download=1'>Download</a>",
           r);
  ap_rputs("</div>", r);
  ap_rputs("</div>", r);
  ap_rputs("<hr />", r);
}