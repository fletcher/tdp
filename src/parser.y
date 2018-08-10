/**

	TDP-Parser -- Parse CSV (and other tabular data) and export to JSON

	@file parser.c

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



//
// Language grammar here
//

%token_type { simple_token * }

%extra_argument { simple_token ** result }

%fallback TEXT_PLAIN NEEDS_ESCAPE.

doc					::= header(B) records(C).							{ *result = B; simple_token_chain_append(B,C); }		// Successful parse with header
doc 				::= records(B).										{ *result = B; }										// Successful parse

eol					::= RECORD_DELIMITER.
eol					::= TDP_EOF.

header				::= record(B).										{ B->type = TDP_HEADER; }

records				::= records(B) record(C).							{ simple_token_chain_append(B, C); }
records				::= record.

record(A)			::= fields(B) eol(C).								{ A = simple_token_new_parent(B, TDP_RECORD); simple_token_free(C); }

fields				::= fields(B) field(C).								{ simple_token_chain_append(B, C); }
fields				::= field.

field(A)			::= contents(B) FIELD_DELIMITER(C).					{ A = simple_token_new_parent(B, TDP_FIELD); simple_token_free(C); if (B->type == TEXT_NUMERIC && B->next == NULL) { A->type = TDP_FIELD_NUMERIC; } }
field(A)			::= contents(B).									{ A = simple_token_new_parent(B, TDP_FIELD); if (B->type == TEXT_NUMERIC && B->next == NULL) { A->type = TDP_FIELD_NUMERIC; } }

contents			::= contents(B) content(C).							{ simple_token_chain_append(B, C); }
contents			::= content.
contents			::= ESCAPED_ESCAPE(B).								{ B->type = TDP_EMPTY_STRING; }							// Empty string

content				::= TEXT_PLAIN.
content				::= TEXT_NUMERIC.
content(A)			::= ESCAPE(B) escaped_contents(C) ESCAPE(D).		{ A = C; simple_token_free(B); simple_token_free(D); }

escaped_contents	::= escaped_contents(B) escaped_content(C).			{ simple_token_chain_append(B, C); }
escaped_contents	::= escaped_content.

escaped_content		::= TEXT_PLAIN.
escaped_content 	::= TEXT_NUMERIC.
escaped_content		::= FIELD_DELIMITER.
escaped_content		::= RECORD_DELIMITER.
escaped_content		::= ESCAPED_ESCAPE.

// %right TEXT_NUMERIC.
// %right RECORD_DELIMITER TDP_EOF.

//
// Additional Configuration
//

%include {
	#include <assert.h>
	#include <stdio.h>
	#include <stdlib.h>

	#include "parser.h"
	#include "reader.h"
	#include "simple_token.h"
}


// Improved error messages for debugging:
//	http://stackoverflow.com/questions/11705737/expected-token-using-lemon-parser-generator


%syntax_error {
#ifndef NDEBUG
	fprintf(stderr,"Parser syntax error.\n");
	int n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
	for (int i = 0; i < n; ++i) {
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) {
			fprintf(stderr,"expected token: %s\n", yyTokenName[i]);
		}
	}
#endif
}


%parse_accept {
//	printf("parsing completed successfully!\n");
}


%parse_failure {
	fprintf(stderr, "Parser failed to successfully parse.\n");
}
