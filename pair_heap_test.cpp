#include <iostream>
#include <cstdlib>
#include <ctime>
#include "pair_heap.h"

void test1()
{
	pair_heap<int> heap;
	srand( time( 0 ) );

	for( int i = 1; i <= 100; i++ ) {
		int tmp = rand();
		heap.insert( tmp, nullptr );
	}

	int prev = -1, tmp;
	while( !heap.empty() ) {
		tmp = heap.delete_min();
		std::cout << tmp << " ";
		if( tmp < prev ) {
			std::cout << "FUCKYOU" << std::endl;
		}
		prev = tmp;
	}
}

void test2()
{
	pair_heap<int> heap;
	for( int i = 1; i <= 100; i++ ) {
		heap.insert( i, nullptr );
	}
	pair_heap<int>::tree_node* ptr101;
	int next = 101;
	heap.insert( next, &ptr101 );
	int m = 102;
	heap.decrease_key( ptr101, m );

	while( !heap.empty() ) {
		int tmp = heap.delete_min();
		std::cout << tmp << " ";
	}
}

int main()
{
	test2();
}
