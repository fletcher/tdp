/**

	TDP-Parser -- Parse CSV (and other tabular data) and export to JSON

	@file lexer.c

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


#include <stdlib.h>

#include "lexer.h"
#include "parser.h"


// Basic scanner struct

#define YYCTYPE		unsigned char
#define YYCURSOR	s->cur
#define YYMARKER	s->ptr
#define YYCTXMARKER	s->ctx

int scan_csv(Scanner * s, const char * stop) {

	scan:

	if (s->cur >= stop) {
		return 0;
	}

	s->start = s->cur;

	/*!re2c
		re2c:yyfill:enable = 0;

		NL								= "\r\n" | '\n' | '\r';
		EOF								= '\x00';


		// Settings for CSV
		[0-9\.]+						{ return TEXT_NUMERIC; }
		EOF								{ return TDP_EOF; }
		NL								{ return RECORD_DELIMITER; }
		','								{ return FIELD_DELIMITER; }
		'""'							{ return ESCAPED_ESCAPE; }
		'"'								{ return ESCAPE; }

		'\\'							{ return NEEDS_ESCAPE; }
		'\t'							{ return NEEDS_ESCAPE; }
		'\b'							{ return NEEDS_ESCAPE; }
		'\f'							{ return NEEDS_ESCAPE; }


		.								{ goto scan; }
	*/
}


int scan_tsv(Scanner * s, const char * stop) {

	scan:

	if (s->cur >= stop) {
		return 0;
	}

	s->start = s->cur;

	/*!re2c
		re2c:yyfill:enable = 0;


		// Settings for TSV
		[0-9\.]+						{ return TEXT_NUMERIC; }
		EOF								{ return TDP_EOF; }
		NL								{ return RECORD_DELIMITER; }
		'\t'							{ return FIELD_DELIMITER; }

		'\\'							{ return NEEDS_ESCAPE; }
		'\b'							{ return NEEDS_ESCAPE; }
		'\f'							{ return NEEDS_ESCAPE; }
		'\"'							{ return NEEDS_ESCAPE; }


		.								{ goto scan; }
	*/
}
