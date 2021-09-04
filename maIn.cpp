#include <iostream>
#include "ThreadPool.hpp"
#include <vector>
#include "CThreadPool.hpp"

using namespace std;

void fun1(int slp, int &value)
{
	value++;
	printf("  hello, fun1 !  %d\n", std::this_thread::get_id());
	if (slp >0)
	{
		printf(" ======= fun1 sleep %d  =========  %d\n", slp, std::this_thread::get_id());
		std::this_thread::sleep_for(std::chrono::milliseconds(slp));
	}
}

int fun2(int n)
{
	printf("  hello, fun1 !  %d\n", std::this_thread::get_id());
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	return n*n;
}

struct gfun {
	int operator()(int n)
	{
		printf(" ======= hello gfun %d  =========  %d\n", n, std::this_thread::get_id());
		return 42;
	}
};

class A
{
public:
	static int AFunc(int n = 0)
	{
		std::cout << n << "  hello, Afun !  " << std::this_thread::get_id() << std::endl;
		return n;
	}

	static std::string Bfun(int n, std::string str, char c) {
		std::cout << n << "  hello, Bfun !  " << str.c_str() << "  " << (int)c << "  " << std::this_thread::get_id() << std::endl;
		 return str;
	}
};

class Test
{
public:
	static void process(int a, int b)
	{
		cout << a << "," << b << ",sum:"<<(a + b) << endl;
	}
};

int main(char* argc, char** argv)
{
	cout << "Hello world!" << endl;

	ThreadPool pool(64);

	std::vector<std::future<int>> results;
	for (int i = 0; i < 10000; ++i)
	{
		results.emplace_back(pool.AddTask([](int x, int y) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			return x + y;
		}, i, i)
		);
	}

	//// 输出线程任务的结果
	for (auto&& result : results)
	{
		std::cout << result.get() << std::endl;
	}

	//auto ff = pool.AddTask(fun1, 0);
	//auto fg = pool.AddTask(gfun(), 0);
	//
	////fg.get();
	//ff.get();
	//fg.get();
	//cout << "Hello world!" << endl;
	//cout << "Hello world!" << endl;
	//cout << "Hello world!" << endl;
	//

	//std::vector<int> values;
	//for (int i=0; i<100; ++i)
	//{
	//	//values.push_back(pool.AddTask(fun2, i).get());
	//	pool.AddTask([i]() {
	//		cout << "hello " << i <<endl;
	//	}).get();
	//}

	//int i = 0;
	//for (auto &vale :values)
	//{
	//	if (i % 10 ==0)
	//	{
	//		cout << endl;
	//	}
	//	cout << vale << " ";
	//	i++;
	//}

	//std::future<std::string> gh = pool.AddTask(&A::Bfun, 999, "1234", 123);
	//gh.get().c_str();
	//int value = 0;
	//for (int i=0; i<50; ++i)
	//{
	//	pool.AddTask(fun1, i, value).get();
	//}
	//cout << value << endl;

	//cout << "======================================" << endl;
	//CThreadPool pool1;
	//pool1.AddTask([]() {
	//	cout << "hello!!!!!" << endl;
	//});

	//Test t1;
	//pool1.AddTask(std::bind(&Test::process, 1, 2));

	system("pause");
	return 0;
}