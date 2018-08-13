/**

	TDP-Parser -- Parse CSV (and other tabular data) and export to JSON

	@file reader.c

	@brief


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2018 Fletcher T. Penney.


	The `c-template` project is released under the MIT License.

	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

		https://github.com/fletcher/MultiMarkdown-4/

	MMD 4 is released under both the MIT License and GPL.


	CuTest is released under the zlib/libpng license. See CuTest.c for the text
	of the license.


	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "d_string.h"
#include "lexer.h"
#include "libTDP.h"
#include "parser.h"
#include "reader.h"
#include "simple_token.h"
#include "stack.h"


#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)


#ifdef TEST
	#include "CuTest.h"
#endif


// Basic parser function declarations
void * ParseAlloc();
void Parse();
void ParseFree();
void ParseTrace();


simple_token * tokenize_text(const char * source, size_t start, size_t len, int format) {

	// Create re2c scanner
	Scanner s;
	s.start = &source[start];		// Start parsing
	s.cur = s.start;

	int type;						// TOKEN type
	simple_token * t = NULL;		// Create token chain
	simple_token * root = simple_token_new(0, start, len);

	// Where do we stop parsing?
	const char * stop = s.start + len;

	// Remember last token to catch other text
	const char * last_stop = s.start;


	do {
		switch (format) {
			case FORMAT_CSV:
				type = scan_csv(&s, stop);
				break;

			case FORMAT_TSV:
				type = scan_tsv(&s, stop);
				break;

			default:
				// Error
				fprintf(stderr, "ERROR.  %d is not a valid parsing format.\n", format);
				return NULL;
				break;
		}

		if (s.start != last_stop) {
			// We skipped characters between tokens

			if (type) {
				t = simple_token_new(TEXT_PLAIN, (size_t)(last_stop - source), (size_t)(s.start - last_stop));
				simple_token_chain_append(root, t);
			} else {
				if (stop > last_stop) {
					// Source text ended without final token

					t = simple_token_new(TEXT_PLAIN, (size_t)(last_stop - source), (size_t)(stop - last_stop));
					simple_token_chain_append(root, t);
				}
			}
		} else if (type == 0 && stop > last_stop) {
			// Source text ended without final token

			t = simple_token_new(TEXT_PLAIN, (size_t)(last_stop - source), (size_t)(stop - last_stop));
			simple_token_chain_append(root, t);
		}

		switch (type) {
			case 0:

				// Source finished
				if (t && t->tail && (t->tail->type != TDP_EOF)) {
					t = simple_token_new(TDP_EOF, (size_t)(s.start - source), (size_t)(s.cur - s.start));
					simple_token_chain_append(root, t);
				}

				break;

			default:
				t = simple_token_new(type, (size_t)(s.start - source), (size_t)(s.cur - s.start));
				simple_token_chain_append(root, t);
				break;
		}

		// Remember where token ends to detect skipped characters
		last_stop = s.cur;
	} while (type != 0);

	return root;
}


int parse_tdp_token_chain(simple_token * chain) {

	// Parser
	void * pParser = ParseAlloc (malloc);
	simple_token * walker = chain->next;
	simple_token * remainder;

	simple_token * result = NULL;

	#ifndef NDEBUG
	fprintf(stderr, "\n");
	ParseTrace(stderr, "parser >>");
	#endif

	while (walker != NULL) {
		remainder = walker->next;

		// Snip token from remainder
		walker->next = NULL;
		walker->tail = walker;

		if (remainder) {
			remainder->prev = NULL;
		}

		Parse(pParser, walker->type, walker, &result);

		walker = remainder;
	}

	// Signal that we're done
	Parse(pParser, 0, NULL, &result);

	ParseFree(pParser, free);

	if (result) {
		// Success
		chain->next = NULL;
		chain->child = result;
		return 0;
	} else {
		// Failed
		return -1;
	}
}


#ifdef TEST
void Test_parse_csv_token_chain(CuTest* tc) {
	DString * test = d_string_new("foo,bar\none,two");
	simple_token * t;
	int result;

	// Valid CSV
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// Valid CSV
	d_string_erase(test, 0, -1);
	d_string_append(test, "foo,\"foo,bar\"");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// Not Valid CSV
	d_string_erase(test, 0, -1);
	d_string_append(test, "foo,\"");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, -1, result);
	simple_token_tree_free(t);

	// Tests from https://github.com/maxogden/csv-spectrum

	// comma_in_quotes.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "first,last,address,city,zip\nJohn,Doe,120 any st.,\"Anytown, WW\",08123");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// empty.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b,c\n1,\"\",\"\"\n2,3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// escaped_quotes.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b\n1,\"ha \"\"ha\"\" ha\"\n3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// json.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "key,val\n1,\"{\"\"type\"\": \"\"Point\"\", \"\"coordinates\"\": [102.0, 0.5]}\"");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// newlines_crlf.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "ka,b,c\r\n1,2,3\r\n\"Once upon \r\na time\",5,6\r\n7,8,9\r\n");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// quotes_and_newlines.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b\n1,\"ha \n\"\"ha\"\" \nha\"\n3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);

	// utf8.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b,c\n1,2,3\n4,5,ʤ");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	CuAssertIntEquals(tc, 0, result);
	simple_token_tree_free(t);
}
#endif


void indent(DString * out, int lev) {
	for (int i = 0; i < lev; ++i) {
		print_const("\t");
	}
}


void export_token_tree_to_json(DString * out, simple_token * t, const char * source, int lev, stack * s, bool array_out);


void export_token_to_json(DString * out, simple_token * t, const char * source, int lev, stack * s, bool array_out) {
	DString * header;
	int count = 0;
	char * text;
	simple_token * c;

	if (t) {
		switch (t->type) {
			case 0:
				print_const("[\n");
				export_token_tree_to_json(out, t->child, source, lev + 1, s, array_out);
				print_const("]\n");
				break;

			case TDP_HEADER:
				c = t->child;

				while (c) {
					header = d_string_new("");
					export_token_tree_to_json(header, c->child, source, 0, s, array_out);
					stack_push(s, header->str);
					d_string_free(header, false);
					c = c->next;
				}

				if (!array_out) {
					break;
				}

			case TDP_RECORD:
				indent(out, lev);

				if (array_out) {
					print_const("[\n");
				} else {
					print_const("{\n");
				}

				c = t->child;

				while (c) {
					indent(out, lev + 1);

					if (!array_out) {
						print_const("\"");
						text = stack_peek_index(s, count);
						print(text);
						print_const("\": ");
					}

					export_token_to_json(out, c, source, lev, s, array_out);

					count++;
					c = c->next;
				}

				indent(out, lev);

				if (array_out) {
					print_const("]");
				} else {
					print_const("}");
				}

				if (t->next && t->next->type == TDP_RECORD) {
					print_const(",\n");
				} else {
					print_const("\n");
				}

				break;

			case TDP_FIELD_NUMERIC:
				export_token_tree_to_json(out, t->child, source, lev, s, array_out);

				if (t->next && (t->next->type == TDP_FIELD || t->next->type == TDP_FIELD_NUMERIC)) {
					print_const(",\n");
				} else {
					print_const("\n");
				}

				break;

			case TDP_FIELD:
				print_const("\"");
				export_token_tree_to_json(out, t->child, source, lev, s, array_out);
				print_const("\"");

				if (t->next && (t->next->type == TDP_FIELD || t->next->type == TDP_FIELD_NUMERIC)) {
					print_const(",\n");
				} else {
					print_const("\n");
				}

				break;

			case TEXT_PLAIN:
			case TEXT_NUMERIC:
			case FIELD_DELIMITER:
				print_token(t);
				break;

			case RECORD_DELIMITER:
				switch (source[t->start]) {
					case '\n':
					case '\r':
						print_const("\\n");
						break;
				}

				break;

			case NEEDS_ESCAPE:
				switch (source[t->start]) {
					case '\b':
						print_const("\\b");
						break;

					case '\f':
						print_const("\\f");
						break;

					case '\n':
						print_const("\\n");
						break;

					case '\r':
						print_const("\\r");
						break;

					case '\t':
						print_const("\\t");
						break;

					case '\"':
						print_const("\\\"");
						break;

					case '\\':
						print_const("\\\\");
						break;

				}

				break;

			case TDP_EMPTY_STRING:
				break;

			case ESCAPED_ESCAPE:
				// TODO: Customize
				print_const("\\\"");
				break;

			default:
				fprintf(stderr, "Error parsing token type %d\n", t->type);
				break;
		}
	}
}


void export_token_tree_to_json(DString * out, simple_token * t, const char * source, int lev, stack * s, bool array_out) {
	while (t) {
		export_token_to_json(out, t, source, lev, s, array_out);

		t = t->next;
	}
}


DString * export_to_json(const char * source, simple_token * tree, bool array_out) {
	DString * out = d_string_new("");
	stack * s = stack_new(5);

	export_token_tree_to_json(out, tree, source, 0, s, array_out);

	return out;
}


#ifdef TEST
void Test_export_to_json(CuTest* tc) {
	DString * test = d_string_new("foo,bar\none,two");
	DString * out;
	simple_token * t;
	int result;

	// Valid CSV
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	simple_token_tree_free(t);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"foo\": \"one\",\n\t\t\"bar\": \"two\"\n\t}\n]\n", out->str);
	d_string_free(out, true);

	// Valid CSV
	d_string_erase(test, 0, -1);
	d_string_append(test, "a\n1");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	simple_token_tree_free(t);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1\n\t}\n]\n", out->str);
	d_string_free(out, true);

	// Boolean
	d_string_erase(test, 0, -1);
	d_string_append(test, "one,two\ntrue,false");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	simple_token_tree_free(t);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"one\": true,\n\t\t\"two\": false\n\t}\n]\n", out->str);
	d_string_free(out, true);

	// Boolean to array of arrays
	d_string_erase(test, 0, -1);
	d_string_append(test, "one,two\ntrue,false");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, true);
	simple_token_tree_free(t);
	CuAssertStrEquals(tc, "[\n\t[\n\t\t\"one\",\n\t\t\"two\"\n\t],\n\t[\n\t\ttrue,\n\t\tfalse\n\t]\n]\n", out->str);
	d_string_free(out, true);

	// Tests from https://github.com/maxogden/csv-spectrum
	// JSON cross-validated with results from https://www.csvjson.com/csv2json

	// comma_in_quotes.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "first,last,address,city,zip\nJohn,Doe,120 any st.,\"Anytown, WW\",08123");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"first\": \"John\",\n\t\t\"last\": \"Doe\",\n\t\t\"address\": \"120 any st.\",\n\t\t\"city\": \"Anytown, WW\",\n\t\t\"zip\": 08123\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// empty.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b,c\n1,\"\",\"\"\n2,3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	simple_token_tree_describe(t, test->str);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1,\n\t\t\"b\": \"\",\n\t\t\"c\": \"\"\n\t},\n\t{\n\t\t\"a\": 2,\n\t\t\"b\": 3,\n\t\t\"c\": 4\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// escaped_quotes.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b\n1,\"ha \"\"ha\"\" ha\"\n3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1,\n\t\t\"b\": \"ha \\\"ha\\\" ha\"\n\t},\n\t{\n\t\t\"a\": 3,\n\t\t\"b\": 4\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// json.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "key,val\n1,\"{\"\"type\"\": \"\"Point\"\", \"\"coordinates\"\": [102.0, 0.5]}\"");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"key\": 1,\n\t\t\"val\": \"{\\\"type\\\": \\\"Point\\\", \\\"coordinates\\\": [102.0, 0.5]}\"\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// newlines_crlf.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b,c\r\n1,2,3\r\n\"Once upon \r\na time\",5,6\r\n7,8,9\r\n");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1,\n\t\t\"b\": 2,\n\t\t\"c\": 3\n\t},\n\t{\n\t\t\"a\": \"Once upon \\na time\",\n\t\t\"b\": 5,\n\t\t\"c\": 6\n\t},\n\t{\n\t\t\"a\": 7,\n\t\t\"b\": 8,\n\t\t\"c\": 9\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// quotes_and_newlines.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b\n1,\"ha \n\"\"ha\"\" \nha\"\n3,4");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1,\n\t\t\"b\": \"ha \\n\\\"ha\\\" \\nha\"\n\t},\n\t{\n\t\t\"a\": 3,\n\t\t\"b\": 4\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	// utf8.csv
	d_string_erase(test, 0, -1);
	d_string_append(test, "a,b,c\n1,2,3\n4,5,ʤ");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_CSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": 1,\n\t\t\"b\": 2,\n\t\t\"c\": 3\n\t},\n\t{\n\t\t\"a\": 4,\n\t\t\"b\": 5,\n\t\t\"c\": \"ʤ\"\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);


	// TSV
	d_string_erase(test, 0, -1);
	d_string_append(test, "a\tb\n\"foo\" \"bar\"\t\"foo\nbar\"\tbat");
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_TSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, false);
	CuAssertStrEquals(tc, "[\n\t{\n\t\t\"a\": \"\\\"foo\\\" \\\"bar\\\"\",\n\t\t\"b\": \"\\\"foo\"\n\t},\n\t{\n\t\t\"a\": \"bar\\\"\",\n\t\t\"b\": \"bat\"\n\t}\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);


	// Export to array of arrays
	t = tokenize_text(test->str, 0, test->currentStringLength, FORMAT_TSV);
	result = parse_tdp_token_chain(t);
	out = export_to_json(test->str, t, true);
	CuAssertStrEquals(tc, "[\n\t[\n\t\t\"a\",\n\t\t\"b\"\n\t],\n\t[\n\t\t\"\\\"foo\\\" \\\"bar\\\"\",\n\t\t\"\\\"foo\"\n\t],\n\t[\n\t\t\"bar\\\"\",\n\t\t\"bat\"\n\t]\n]\n", out->str);
	simple_token_tree_free(t);
	d_string_free(out, true);

	d_string_free(test, true);
}
#endif


/// Convert CSV text to JSON
DString * csv_to_json(DString * source, bool array_out) {
	simple_token * t = tokenize_text(source->str, 0, source->currentStringLength, FORMAT_CSV);
	parse_tdp_token_chain(t);
	DString * json = export_to_json(source->str, t, array_out);

	return json;
}


/// Convert TSV text to JSON
DString * tsv_to_json(DString * source, bool array_out) {
	simple_token * t = tokenize_text(source->str, 0, source->currentStringLength, FORMAT_TSV);
	parse_tdp_token_chain(t);
	DString * json = export_to_json(source->str, t, array_out);

	return json;
}
