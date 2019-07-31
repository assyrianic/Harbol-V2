#!/bin/bash
cd "$(dirname "$0")"

gcc -Wall -Wextra -std=c99 -g -O2 -shared -c test_harbol_plugins/test_plugin.c
gcc -shared -o test_plugin.so test_plugin.o

gcc -Wall -Wextra -std=c99 -g -O2 -shared -c test_harbol_plugins/plugin_subfolder/plugin.c
gcc -shared -o plugin.so plugin.o

rm test_plugin.o plugin.o
mv --force plugin.so test_harbol_plugins/plugin_subfolder
mv --force test_plugin.so test_harbol_plugins/

gcc -Wall -Wextra -pedantic -std=c99 -g -O2 test_suite.c -L. -lharbol -o harbol_testprogram -ldl -Wl,--export-dynamic
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./harbol_testprogram

# for SERIOUS debugging!
#--vgdb-error=0 

rm test_harbol_plugins/test_plugin.so test_harbol_plugins/plugin_subfolder/plugin.so
