#include "fib_heap.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using std::swap;

#define EXPECT_TRUE( expr ) \
	do {\
		if( !(expr) ) {\
			std::cerr << "Boom in line " << __LINE__ << std::endl;\
			throw 0;\
		}\
	} while(0)

#define	EXPECT_EQ_INT(expect, actual) \
	do {\
		if( (expect) != (actual) ) {\
			std::cerr << "Expect " << expect << " But actually " << actual << std::endl;\
			throw 0;\
		}\
	} while(0)

template<typename T>
void expect_equal( const T& a, const T& b )
{
	if( a != b ) {
		std::cerr << "Expect " << a << " But actually " << b << std::endl;
		throw 0;
	}
}

const int testsz = 100000;

void test1()
{
	// test push, pop and top
	fib_heap<int> heap1;
	std::vector<int> vec( testsz );
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

	for( int i = 0; i < testsz; ++i ) {
		auto res = dist( eng );
		vec[i] = res;
		heap1.push( res );
	}
	heap1.clear();
	EXPECT_TRUE( heap1.empty() );
	for( auto a : vec ) {
		heap1.push( a );
	}

	EXPECT_EQ_INT( testsz, heap1.size() );
	std::sort( vec.begin(), vec.end() );
	fib_heap<int> heap2( heap1 );
	fib_heap<int> heap3( std::move( heap2 ) );
	fib_heap<int> heap4;
	heap4 = heap3;
	fib_heap<int> heap5;
	heap5 = std::move( heap4 );
	fib_heap<int> heap6;
	swap( heap5, heap6 );

	for( int i = 0; i < testsz; ++i ) {
		EXPECT_EQ_INT( vec[i], heap3.top() );
		EXPECT_EQ_INT( vec[i], heap6.top() );
		heap3.pop();
		heap6.pop();
	}
}

void test2()
{
	// test decrease_key
	fib_heap<int> heap1, heap2, heap3;
	std::vector<int> vec( testsz );
	std::vector<fib_heap<int>::ptr> ptrvec( testsz );
	std::default_random_engine eng;
	std::uniform_int_distribution<int> dist;

	for( int i = 0; i < testsz / 2; ++i ) {
		auto res = dist( eng );
		vec[i] = res;
		ptrvec[i] = heap1.push( res );
	}
	for( int i = testsz / 2; i < testsz; ++i ) {
		auto res = dist( eng );
		vec[i] = res;
		ptrvec[i] = heap2.push( res );
	}

	heap3 = merge( heap1, heap2 );

	std::sort( vec.begin(), vec.end() );

	for( auto p : ptrvec ) {
		heap3.decrease_key( p, p.val() - 10 );
	}

	for( int i = 0; i < testsz; ++i ) {
		EXPECT_EQ_INT( vec[i] - 10, heap3.top() );
		heap3.pop();
	}
}

int main()
{
	test1();
	test2();
}
