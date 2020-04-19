#!/usr/bin/bash

TEST_COUNT=0
function do_test
{
	cd $1
	printf "["
	[[ $(./run.sh) ]] && 
		printf "failed" || 
		printf "passed"
	printf "] $1\n"
	cd ..
	TEST_COUNT=$(( $TEST_COUNT+1 ))
}

do_test keywords
do_test or
do_test optional
do_test label
do_test label_optional
do_test sub_node
do_test sub_node_recursive
do_test sub_node_if
do_test combine_rules
echo "Ran $TEST_COUNT test(s)"

