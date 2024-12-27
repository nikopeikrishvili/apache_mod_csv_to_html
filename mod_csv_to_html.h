#pragma once
const char *get_file_name(char *filename);
void add_styles(request_rec *r);
void render_table(const char *file_location, request_rec *r);
int parse_csv_line(const char *line, char **fields);
void get_file_info_header(const char *file_location, request_rec *r);
char *get_arg_value(const char *args, const char *field);