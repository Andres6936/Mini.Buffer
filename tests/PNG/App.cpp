#include <SPNG/spng.h>
#include <MiniFB.h>

#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <vector>

class App
{

private:

	bool isRunning = true;
	mfb_window* window{ nullptr };
	std::vector <unsigned char> buffer;

public:

	App()
	{
		window = mfb_open_ex("my display", 800, 600, WF_RESIZABLE);
		buffer.resize(800 * 600 * 4);
		std::fill(buffer.begin(), buffer.end(), 255);
	}

	~App()
	{
		window = nullptr;
	}


	int run(int argc, char** argv)
	{
		if (argc < 2)
		{
			throw std::exception("No Input File for Read");
		}

		do
		{
			int state = mfb_update_ex(window, buffer.data(), 800, 600);

			if (state < 0)
			{
				isRunning = false;
			}


			// Note that, by default, if ESC key is pressed mfb_update / mfb_update_ex
			// will return -1 (and the window will have been destroyed internally).
		} while (mfb_wait_sync(window) && isRunning);

		return 0;
	}

};

int main(int argc, char** argv)
{
	try
	{
		return (new App())->run(argc, argv);
	}
	catch (std::exception& exception)
	{
		std::cout << "Exception Message: " << exception.what();
		return -1;
	}
}