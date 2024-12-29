#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "apr_strings.h"
#include "mod_csv_to_html.h"
#include "csv_reader.h"

/**
 * Main handler for module
 * @param r - Apache request structure
 * @return OK or DECLINED , in case of DECLINED file will be downloaded
 */
static int csv_to_html_handler(request_rec *r) {
    if (strcmp(r->handler, "csv_to_html") != 0) {
        return DECLINED;
    }
    if (r->args) {
        const char *has_download_arg = getArgValues(r->args, "download", r);
        if (has_download_arg != NULL && *has_download_arg == '1') {
            return DECLINED; // Download file
        }
    }
    r->content_type = "text/html";
    const char *csv_filename = apr_pstrdup(r->pool, r->canonical_filename);
    if (csv_filename == NULL) {
        return DECLINED; // Download file
    }
    const char *filename = getFileName(csv_filename);
    FILE *pt_file = openFile(r->canonical_filename, "r");
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
    closeFile(pt_file);

    return OK;
}

static void csv_to_html_register_hooks(apr_pool_t *p) {
    ap_hook_handler(csv_to_html_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA csv_to_html_module = {
    STANDARD20_MODULE_STUFF,
    NULL, /* create per-dir    config structures */
    NULL, /* merge  per-dir    config structures */
    NULL, /* create per-server config structures */
    NULL, /* merge  per-server config structures */
    NULL, /* table of config file commands       */
    csv_to_html_register_hooks /* register hooks                      */
};

/**
 * Given file path it returns just filename and extension
 * @param pt_filePath - file full pack
 * @return
 */
const char *getFileName(const char *pt_filePath) {
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
void addStyles(request_rec *r) {
    ap_rputs("<style>\n", r);
    ap_rputs(
        "table {width:100%;border-collapse: collapse;margin: 25px 0;font-size: 0.9em;font-family: sans-serif;box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);}\n",
        r);
    ap_rputs("table thead tr {background-color: #009879;color: #ffffff; text-align: left;}\n", r);
    ap_rputs("th, td {padding: 12px 15px;}\n", r);
    ap_rputs("table tbody tr{border-bottom: 1px solid #dddddd;}\n", r);
    ap_rputs("table tbody tr:nth-of-type(even){background-color: #f3f3f3;}\n", r);
    ap_rputs("table tbody tbody tr:last-of-type{border-bottom: 2px solid #009879;}\n", r);
    ap_rputs("</style>", r);
}

/**
 *
 * @param pt_file - opened file pointer - file descriptor is not NULL it's checked in parent function
 * @param r - Apache request structure
 */
void renderTable(FILE *pt_file, request_rec *r) {
    ap_rputs("<table>", r);
    ap_rputs("<tbody>", r);
    const char delimiter = getDelimiter(pt_file);

    const int fieldsCount = getFieldsCount(pt_file, &delimiter);
    char line[LINE_MAX];
    rewind(pt_file);
    while (fgets(line, LINE_MAX, pt_file)) {
        ap_rputs("<tr>", r);
        const char *csv_line = apr_pstrdup(r->pool, line);
        char *fields[fieldsCount];
        parseCsvLine(csv_line, &delimiter, &fieldsCount, fields, r);
        for (int i = 0; i < fieldsCount; i++) {
            ap_rputs("<td>", r);
            ap_rputs(fields[i], r);
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
void getFileInfoHeader(const char *pt_fileLocation, request_rec *r) {
    ap_rputs(
        "<div style='align-items: center; display:flex; flex-direction:row; justify-content: space-between;'>",
        r);
    ap_rputs("<div>", r);
    ap_rputs("<h2>", r);
    ap_rputs(pt_fileLocation, r);
    ap_rputs("</h2>", r);
    ap_rputs("</div>", r);

    ap_rputs("<div style='cursor:pointer;'>", r);
    ap_rputs(
        "<a style='border-radius:5px; text-decoration:none; font-family: sans-serif; background-color: DodgerBlue; border: none; color: white; padding: 12px 30px; cursor: pointer; font-size: 20px;' href='?download=1'>Download</a>",
        r);
    ap_rputs("</div>", r);
    ap_rputs("</div>", r);
    ap_rputs("<hr />", r);
}

/**
 *
 * @param pt_args - Arguments string
 * @param pt_field - field that we are looking for in arguments
 * @param r - Apache request object
 * @return
 */
char *getArgValues(const char *pt_args, const char *pt_field, const request_rec *r) {
    if (pt_args == NULL || pt_field == NULL) {
        return NULL;
    }

    size_t field_len = strlen(pt_field);
    const char *start = pt_args;

    while (start && *start != '\0') {
        // Find the position of the '=' in the current key-value pair
        const char *equals = strchr(start, '=');
        if (equals == NULL) {
            break;
        }

        // Check if the key matches the requested field
        if ((size_t) (equals - start) == field_len && strncmp(start, pt_field, field_len) == 0) {
            // Found the key; extract the value
            const char *value_start = equals + 1;
            const char *amp = strchr(value_start, '&'); // Find the end of the value
            size_t value_len = amp ? (size_t) (amp - value_start) : strlen(value_start);

            char *value = apr_pcalloc(r->pool, value_len + 1);
            if (value == NULL) {
                return NULL; // Memory allocation failed
            }

            strncpy(value, value_start, value_len);
            value[value_len] = '\0';
            return value;
        }

        // Move to the next key-value pair
        start = strchr(start, '&');
        if (start) {
            start++; // Skip the '&'
        }
    }

    return NULL; // Field not found
}
