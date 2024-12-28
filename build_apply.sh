#!/bin/bash
apxs -iac mod_csv_to_html.c csv_reader.c && apachectl restart