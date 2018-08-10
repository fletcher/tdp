/**

	TDP-Parser -- Parse CSV (and other tabular data) and export to JSON

	@file simple_token.h

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


#ifndef SIMPLE_TOKEN_TDP_PARSER_H
#define SIMPLE_TOKEN_TDP_PARSER_H

#include <stdlib.h>

struct simple_token {
	unsigned short				type;			//!< Type for the token

	size_t						start;			//!< Starting offset in the source string
	size_t						len;			//!< Length of the token in the source string

	struct simple_token *		next;			//!< Pointer to next token in the chain
	struct simple_token *		prev;			//!< Pointer to previous marker in the chain
	struct simple_token *		child;			//!< Pointer to child chain

	struct simple_token *		tail;			//!< Pointer to last token in the chain
};

typedef struct simple_token simple_token;


/// Get pointer to a new token
simple_token * simple_token_new(
	unsigned short type,						//!< Type for new token
	size_t start,								//!< Starting offset for token
	size_t len									//!< Len of token
);

/// Add a new token to the end of a token chain.  The new token
/// may or may not also be the start of a chain
void simple_token_chain_append(
	simple_token * chain_start,					//!< Pointer to start of token chain
	simple_token * t							//!< Pointer to token to append
);

/// Free token
void simple_token_free(
	simple_token * t							//!< Pointer to token to be freed
);

/// Free token tree
void simple_token_tree_free(
	simple_token * t							//!< Pointer to token to be freed
);

/// Create a parent for a chain of tokens
simple_token * simple_token_new_parent(
	simple_token * child,						//!< Pointer to child token chain
	unsigned short type							//!< Type for new token
);


/// Print a description of the token based on specified string
void simple_token_describe(
	simple_token * t,							//!< Pointer to token to described
	const char * string							//!< Source string
);

/// Print a description of the token tree based on specified string
void simple_token_tree_describe(
	simple_token * t,							//!< Pointer to token to described
	const char * string							//!< Source string
);

#endif
