#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

const int CACHE_LINE_SIZE = 64;

constexpr int T_SIZE = 100000000;
short rand_arr[T_SIZE];

#define abs(x) (((x)>0)?(x):-(x))	

int abs2(int x)	
{
	int y = x >> 31;
	return (y ^ x) - y;
}

int main()
{
	// 시스템 call을 통해 시간을 측정
	//volatile long long tmp = 0;
	//auto start = high_resolution_clock::now();
	//for (int j = 0; j < 10000000; ++j) {
	//	tmp += j;
	//	this_thread::yield();	// 
	//}
	//auto duration = high_resolution_clock::now() - start;
	//cout << "Time " << duration_cast<milliseconds>(duration).count();
	//cout << " msec\n";
	//cout << "RESULT " << tmp << endl;

	// 캐시 미스
	//for (int i = 0; i < 20; ++i) {
	//	const int size = 1024 << i;
	//	char* a = (char*)malloc(size);
	//	unsigned int index = 0;
	//	int tmp = 0;
	//	auto start = high_resolution_clock::now();
	//	for (int j = 0; j < 100000000; ++j) {
	//		tmp += a[index % size];
	//		index += CACHE_LINE_SIZE * 11;
	//	}
	//	auto dur = high_resolution_clock::now() - start;
	//	cout << "Size : " << size / 1024 << "K, ";
	//	cout << "Time " << duration_cast<milliseconds>(dur).count();
	//	cout << " msec " << tmp << endl;
	//}




	for (int i = 0; i < T_SIZE; ++i) rand_arr[i] = rand() - 16384;
	int sum = 0;
	auto start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs(rand_arr[i]);
	auto du = high_resolution_clock::now() - start_t;
	cout << "[abs] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;
	sum = 0;
	start_t = high_resolution_clock::now();
	for (int i = 0; i < T_SIZE; ++i) sum += abs2(rand_arr[i]);
	du = high_resolution_clock::now() - start_t;
	cout << "[abs2] Time " << duration_cast<milliseconds>(du).count() << " ms\n";
	cout << "Result : " << sum << endl;


}