#!/bin/sh

echo '------------------------------------------------------------------------------------------'
echo
echo '                 Running example operations on horizontal image'
echo
echo '------------------------------------------------------------------------------------------'
echo
echo '  1. Resize proportionally to 230px width w/ prefiltering & sharpening (beautiful, but slow)'
echo '  2. Resize proportionally to 640px width'
echo '  3. Resize proportionally to 800px width and preserve original metadata'
echo '  4. Read the photos original metadata'
echo '------------------------------------------------------------------------------------------'
echo
echo 'Output: '
echo 

example_1_input=`cat example-1.json`

../build/arion --input "$example_1_input"

if [ $? -ne 0 ] ; then echo "Failed example 1"; exit 1; fi

echo '------------------------------------------------------------------------------------------'
echo
echo '                  Example 2: Running resize operations on vertical image'
echo
echo '------------------------------------------------------------------------------------------'
echo 
echo '  1. Resize proportionally to 150px width'
echo '  2. Resize and crop to a 150 x 150 px square'
echo '  3. Resize proportionally to 300px height'
echo '  4. Resize proportionally to 800px width, apply watermark, and preserve metadata*'
echo '  5. Copy the original and preserve metadata*'
echo
echo '     *In addition to preserving original metadata some overrides are provided using the'
echo '      write_meta parameter'
echo
echo '------------------------------------------------------------------------------------------'
echo
echo 'Output: '
echo 

example_2_input=`cat example-2.json`

../build/arion --input "$example_2_input"

if [ $? -ne 0 ] ; then echo "Failed example 2"; exit 1; fi

echo '------------------------------------------------------------------------------------------'
echo
echo '             Example 3: Read back the meta data written by previous command'
echo
echo '------------------------------------------------------------------------------------------'
echo
echo 'Output: '
echo 

example_3_input=`cat example-3.json`

../build/arion --input "$example_3_input"

if [ $? -ne 0 ] ; then echo "Failed example 3"; exit 1; fi

