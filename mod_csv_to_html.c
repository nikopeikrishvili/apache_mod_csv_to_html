#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "mod_csv_to_html.h"
/* The sample content handler */


static int csv_to_html_handler(request_rec *r) {
    if (strcmp(r->handler, "csv_to_html")) {
        return DECLINED;
    }

    if (r->args) {
        char *hasDownload = get_arg_value(r->args, "download");
        if (hasDownload != NULL && *hasDownload == '1') {
            free(hasDownload);
            return DECLINED;
        }
    }
    r->content_type = "text/html";
    char *csv_filename = malloc(strlen(r->canonical_filename) + 1);
    if (csv_filename == NULL) {
        csv_filename = "file_not_found.csv";
    } else {
        strcpy(csv_filename, r->canonical_filename);
    }
    const char *filename = get_file_name(csv_filename);
    ap_rputs("<!DOCTYPE html><html><head><title>",r);
    ap_rputs(filename, r);
    ap_rputs("</title>", r);
    add_styles(r);
    ap_rputs("</head><body style='padding:10px 20px;'>", r);
    get_file_info_header(filename, r);
    if (strcmp(csv_filename,"file_not_found.csv") == 0) {
        free(csv_filename);
    }
    render_table(r->canonical_filename, r);
    ap_rputs("</body>", r);
    ap_rputs("</html>", r);
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

const char *get_file_name(char *path) {
    // Find the last slash
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        return last_slash + 1; // Skip the slash
    }
    return path;
}

void add_styles(request_rec *r) {
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

void render_table(const char *file_location, request_rec *r) {
    ap_rputs("<table>", r);
    ap_rputs("<tbody>", r);
    FILE *stream = fopen(file_location, "r");
    if (NULL != stream) {
        char line[1024];
        while (fgets(line, 1024, stream)) {
            ap_rputs("<tr>", r);
            char *csv_line = strdup(line);
            char *fields[100];
            int field_count = parse_csv_line(csv_line, fields);
            for (int i = 0; i < field_count; i++) {
                ap_rputs("<td>", r);
                ap_rputs(fields[i], r);
                ap_rputs("</td>", r);
                free(fields[i]);
            }
            ap_rputs("</tr>", r);
        }
        fclose(stream);
    } else {
        ap_rputs("File not found", r);
    }

    ap_rputs("</tbody>", r);
    ap_rputs("</table>", r);
}

/**
 * Parses a single line of CSV data into an array of fields.
 *
 * @param line The CSV line to parse.
 * @param fields Array to store pointers to parsed fields.
 * @return The number of fields parsed.
 */
int parse_csv_line(const char *line, char **fields) {
    const char *ptr = line;
    int field_count = 0;
    bool in_quotes = false;
    char buffer[1024];
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
            if (*ptr == ',') {
                // End of field
                buffer[buffer_pos] = '\0';
                fields[field_count++] = strdup(buffer);
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
    fields[field_count++] = strdup(buffer);

    return field_count;
}

void get_file_info_header(const char *filename, request_rec *r) {
    ap_rputs(
        "<div style='align-items: center; display:flex; flex-direction:row; justify-content: space-between;'>",
        r);
    ap_rputs("<div>", r);
    ap_rputs("<h2>", r);
    ap_rputs(filename, r);
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
 * Parses the args string and retrieves the value for a given field name.
 *
 * @param args The query string in the format "key=value&key=value".
 * @param field The field name whose value is to be retrieved.
 * @return The value of the field, or NULL if the field is not found.
 *         The caller is responsible for freeing the returned value.
 */
char *get_arg_value(const char *args, const char *field) {
    if (args == NULL || field == NULL) {
        return NULL;
    }

    size_t field_len = strlen(field);
    const char *start = args;

    while (start && *start != '\0') {
        // Find the position of the '=' in the current key-value pair
        const char *equals = strchr(start, '=');
        if (equals == NULL) {
            break; // Malformed query string
        }

        // Check if the key matches the requested field
        if ((size_t) (equals - start) == field_len && strncmp(start, field, field_len) == 0) {
            // Found the key; extract the value
            const char *value_start = equals + 1;
            const char *amp = strchr(value_start, '&'); // Find the end of the value
            size_t value_len = amp ? (size_t) (amp - value_start) : strlen(value_start);

            char *value = malloc(value_len + 1);
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
