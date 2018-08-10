## About ##

|            |                           |  
| ---------- | ------------------------- |  
| Title:     | TDP-Parser        |  
| Author:    | Fletcher T. Penney       |  
| Date:      | 2018-08-10 |  
| Copyright: | Copyright © 2018 Fletcher T. Penney.    |  
| Version:   | 1.0.0      |  


## Introduction ##

TDP (Tabular Data Parser) is designed to make it easy to convert several types
of tabular data into JSON data.

For example, the following CSV (Comma-separated values):

	foo,bar
	one,1
	two,2

can be converted into the following JSON:

	[
		{
			"foo": "one",
			"bar": 1
		},
		{
			"foo": "two",
			"bar": 2
		}
	]

`tdp` can also process TSV (Tab-separated values).


## Usage ##

Convert CSV into JSON:

	tdp data.csv > data.json

Convert TSV:

	tdp -f tdp > data.json


## Why? ##

While this utility can be quite useful in itself, my purpose in creating it
was to allow even greater flexibility when using
[Magnum](https://github.com/fletcher/magnum).  Magnum is a C implementation of
the [Mustache] templating system.

You can think of Magnum as "mail merge" for all sorts of documents.  You
create a document template, and then use JSON data to control how the template
is filled in.  However, it's usefulness is not limited to simply filling in
letters with names and addresses.  You can create arbitrarily powerful
templates for any sort of text document:

* Plain text
* HTML
* Markdown/MultiMarkdown documents

But in addition to text documents, you can even use Magnum to process
templates of source code.  For example, if you have written boilerplate code
to create a SQLite table, you can reuse the same template just by filling it
in with different data.  Even better, when you later make an improvement or
fix a bug, that same fix can be applied everywhere by simply processing the
new template with the original data to rebuild the final files.

Since creating Magnum, I have used it to reduce programming time associated on
several projects, easily saving a hundred hours of time writing similar chunks
of code, and applying fixes across multiple projects.


In the same way that it is much easier and faster to write MultiMarkdown than
to hand-code HTML, using this templating approach to programming and other
projects can save you huge amounts of time.


## OK, so how does TDP help with Magnum? ##

Magnum is limited to using JSON-formatted data to complete the templates.  TDP
makes it easy to quickly convert other common data formats into JSON.  This
means that you can use data in Excel, save it as CSV, and then quickly convert
it to JSON for use with Magnum.  Or you can export data from SQLite in a CSV
format for the same purpose.


## Why the name TDP? ##

Because I couldn't think of anything creative.  I'm open to changing it for a
better one.


## Technical Information ##

I've tried to ensure an accurate parser for the CSV and TSV formats, and there
are included unit tests for accuracy.  There are currently more rigorous tests
for the CSV format, which has more edge cases than TSV.

Some of the CSV unit tests come from [csv-spectrum].

If you find examples that are improperly parsed, then feel free to let me
know.  Do keep in mind that the parser is designed to be compliant with the
CSV and TSV "specs" (with the understanding that they lack true formal
specifications), and the parser does not support some of the variations used
by some programs.

* CSV ["pseudo-spec"](https://tools.ietf.org/html/rfc4180)
* TSV ["pseudo-spec"](https://www.iana.org/assignments/media-types/text/tab-separated-values)


[Mustache]:	https://mustache.github.io/
[csv-spectrum]:	https://github.com/maxogden/csv-spectrum
