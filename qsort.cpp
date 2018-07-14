#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cassert>

template<typename Iterator>
void qsort1(Iterator b, Iterator e)
{
	using std::swap;
	
	if (b >= e) {
		return;
	}
	
	Iterator geb = b, unb = b;
	auto pivot = std::prev(e);
	while (unb != pivot) {
		if (*unb < *pivot) {
			swap(*unb, *geb);
			++geb;
		}
		++unb;
	}
	swap(*geb, *pivot);
	
	qsort1(b, geb);
	qsort1(std::next(geb), e);
}

template<typename Iterator>
void qsort2(Iterator b, Iterator e)
{
	using std::swap;

	if (b >= e) {
		return;
	}

	Iterator eqb = b, gtb = b, index = b;
	auto pivot = std::prev(e);
	while (index != pivot) {
		if (*index < *pivot) {
			swap(*index, *eqb);
			if (eqb != gtb)
				swap(*index, *gtb);
			++eqb;
			++gtb;			
		} else if (*pivot < *index) {
			//
		} else {
			swap(*index, *gtb);
			++gtb;
		}
		++index;
	}
	
	swap(*gtb, *pivot);
	++gtb;
	
	qsort2(b, eqb);
	qsort2(gtb, e);
}

class TimerGuard
{
public:
	TimerGuard() {
		begin = std::chrono::system_clock::now();
	}
	~TimerGuard() {
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed = end - begin;
		std::cout << elapsed.count() << "s\n";
	}
private:
	std::chrono::system_clock::time_point begin;
};

int main()
{
	constexpr size_t VSIZE = 100000;
	std::vector<int> v(VSIZE);
	std::random_device dev;
	std::mt19937 g(dev());
	std::default_random_engine e(dev());
	std::uniform_int_distribution<int> dist(0, 100000000);
	for (auto& i: v) {
		i = dist(e);
	}
	std::shuffle(v.begin(), v.end(), g);
	std::cout << std::endl;

	auto v3 = v;
	{
		TimerGuard tg;
		qsort1(v.begin(), v.end());
		if (std::is_sorted(v.begin(), v.end())) {
			std::cout << "sorted\n";
		} else {
			std::cout << "unsorted\n";
		}
	}

	std::vector<int> v2(VSIZE, 1);
	auto v4 = v2;
	{
		TimerGuard tg;
		qsort1(v2.begin(), v2.end());
		if (std::is_sorted(v2.begin(), v2.end())) {
			std::cout << "sorted\n";
		} else {
			std::cout << "unsorted\n";
		}
	}

	{
		TimerGuard tg;
		qsort2(v3.begin(), v3.end());
		if (std::is_sorted(v3.begin(), v3.end())) {
			std::cout << "sorted\n";
		} else {
			std::cout << "unsorted\n";
			std::for_each(v3.begin(), v3.end(), [](const auto& i){std::cout << i << ' ';});
		}
	}

	{
		TimerGuard tg;
		qsort2(v4.begin(), v4.end());
		if (std::is_sorted(v4.begin(), v4.end())) {
			std::cout << "sorted\n";
		} else {
			std::cout << "unsorted\n";
		}
	}
	
}
