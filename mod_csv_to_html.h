#pragma once
/**
*
*/
typedef struct {
    int enabled;
    char context[256];
    char *css_path;
    char *js_path;
    int has_header;
} csv_to_html_config;
static const char *getFileName(const char *pt_filePath);
static void addStyles(request_rec *r, const csv_to_html_config *config);
static void renderTable(FILE *pt_file, request_rec *r);
static void getFileInfoHeader(const char *pt_fileLocation, request_rec *r, const csv_to_html_config *config);
static const char *configCsvToHtmlEnabled(cmd_parms *cmd, void *cfg, const char *arg);
static const char *configCsvToHtmlHasHeader(cmd_parms *cmd, void *cfg, const char *arg);
static void csv_to_html_register_hooks(apr_pool_t *p);
void *create_dir_conf(apr_pool_t *pool, char *context);
void *merge_dir_conf(apr_pool_t *pool, void *BASE, void *ADD);