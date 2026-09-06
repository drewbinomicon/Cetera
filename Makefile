test:
	g++ -std=c++17 test_cetera.cpp -o test_cetera -I/usr/include/tcl8.6 -ltcl8.6 -ltk8.6 && ./test_cetera

clean:
	rm -f test_cetera a.out
