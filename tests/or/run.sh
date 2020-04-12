#!/usr/bin/bash

function run_lang
{
	tinylex *.tinylex -l $1 -o lexer.h > /dev/null
	tinyparse *.tinyparse -l $1 -o parser.h -e Entry
	$2 -I./ ../$3 -o test
	./test > got.txt

	diff got.txt expected.txt
	rm parser.h lexer.h test got.txt
}

run_lang c gcc test.c

