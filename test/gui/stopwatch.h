#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

class Stopwatch
{
public:

	using clock = std::chrono::steady_clock;
	Stopwatch(std::string_view s)
	{
		name = s;
		std::cerr << name << " enter\n";
	}

	void stop()
	{
		std::cerr << name << " leave (" << std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start).count() << "ms)\n";
		running = false;
	}

	~Stopwatch()
	{
		if (running)
		{
			stop();
		}
	}
	
	bool running = true;
	std::string name;
	clock::time_point start = clock::now();
};