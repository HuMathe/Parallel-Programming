all: p_sort s_sort data checker
.PHONY: all


p_sort: odd_even_parallel.cpp
	mpic++ odd_even_parallel.cpp -o p_sort -std=c++11 -w

s_sort: odd_even_serial.cpp
	g++ odd_even_serial.cpp -o s_sort -std=c++11 -w

data: data.cpp
	g++ data.cpp -o data

checker: check_sorted.cpp
	g++ check_sorted.cpp -o checker

clean:
	rm p_sort s_sort data checker data.in data.*.out