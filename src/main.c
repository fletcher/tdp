/*

	main.c -- Template main()

	Copyright © 2015-2016 Fletcher T. Penney.


	This program is free software you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argtable3.h"
#include "d_string.h"
#include "file.h"
#include "libTDP.h"

// argtable structs
struct arg_lit *a_help;
struct arg_str *a_format;
struct arg_end *a_end;
struct arg_file *a_file;

void convert_buffer(DString * buffer, short format) {
	if (buffer) {
		DString * out = d_string_new("");

		switch (format) {
			case FORMAT_CSV:
				out = csv_to_json(buffer);
				break;

			case FORMAT_TSV:
				out = tsv_to_json(buffer);
				break;
		}

		if (out) {
			fwrite(out->str, out->currentStringLength, 1, stdout);
		}

		d_string_free(out, true);
	}
}


int main( int argc, char** argv ) {
	char * binname = "tdp";
	short format = FORMAT_CSV;
	int exitcode = EXIT_SUCCESS;

	void *argtable[] = {
		a_help			= arg_lit0(NULL, "help", "display this help and exit"),

		a_format		= arg_str0("f", "from", "FORMAT", "convert from tabular data format (default CSV), FORMAT = csv|tsv"),

		a_file 			= arg_filen(NULL, NULL, "<FILE>", 0, argc + 2, "read input from file(s) -- use stdin if no files given"),

		a_end 			= arg_end(20),
	};

	int nerrors = arg_parse(argc, argv, argtable);

	if (a_help->count > 0) {
		printf("\n\tUsage: %s", binname);
		arg_print_syntax(stdout, argtable, "\n\n");
		printf("Options:\n");
		arg_print_glossary(stdout, argtable, "\t%-25s %s\n");
		printf("\n");
		exitcode = 1;
		goto exit;
	}

	if (nerrors > 0) {
		arg_print_errors(stdout, a_end, binname);
		exitcode = 1;
		goto exit;
	}

	if (a_format->count > 0) {
		if (strcmp(a_format->sval[0], "csv") == 0) {
			format = FORMAT_CSV;
		} else if (strcmp(a_format->sval[0], "tsv") == 0) {
			format = FORMAT_TSV;
		} else {
			fprintf(stderr, "%s: Unknown input format '%s'\n", binname, a_format->sval[0]);
			exitcode = 1;
			goto exit;
		}
	}

	DString * buffer;

	if (a_file->count == 0) {
		// Read from stdin
		buffer = stdin_buffer();
		convert_buffer(buffer, format);
		d_string_free(buffer, true);
	} else {
		// Read from files
		for (int i = 0; i < a_file->count; ++i) {
			buffer = scan_file(a_file->filename[i]);

			if (buffer == NULL) {
				fprintf(stderr, "Error reading file '%s'\n", a_file->filename[i]);
				exitcode = 1;
				goto exit;
			}

			convert_buffer(buffer, format);
			d_string_free(buffer, true);
		}
	}

exit:
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	return exitcode;
}
