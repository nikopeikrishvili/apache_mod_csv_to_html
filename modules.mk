mod_csv_to_html.la: mod_csv_to_html.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_csv_to_html.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_csv_to_html.la
