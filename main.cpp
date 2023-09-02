#include "main.h"

int init()
{
	try
	{
		SetConsoleCP(1251);
		SetConsoleOutputCP(1251);
		Searcher searcher{};
		searcher.Run();
	}
	catch (std::exception& ex)
	{
		std::cerr << "Error!" << ex.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main()
{
	return init();
}
