/**

	TDP-Parser -- Parse CSV (and other tabular data) and export to JSON

	@file libTDP.h

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


#ifndef LIBTDP_PARSER_H
#define LIBTDP_PARSER_H

#include <stdbool.h>


/// typedefs for internal data structures.  If you intend to work with these structures
/// in your own code, you may need to import additional header files.

/// From d_string.h:
typedef struct DString DString;


// Input formats
enum parser_formats {
	FORMAT_CSV,
	FORMAT_TSV
};


/// Convert CSV to JSON
DString * csv_to_json(DString * source, bool array_out);


/// Convert TSV text to JSON
DString * tsv_to_json(DString * source, bool array_out);


#endif
