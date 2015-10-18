#!/bin/sh

echo
echo 'Running example operations on horizontal image'
echo

example_1_input=`cat example-1.json`

../build/resizer --input "$example_1_input"

example_2_input=`cat example-2.json`

echo
echo 'Running example operations on vertical image'
echo

../build/resizer --input "$example_2_input"

