#pragma once
#include "Header.h"
class menu
{
public:
	menu(std::list<std::string> arr, std::function<void()> funcs[menusize])
		: current(0), _size(arr.size())
	{
		unsigned short i = 0;
		for (; i < menusize; i++)
			_funcs[i] = funcs[i];
		i = 0;
		for (auto&& lnk : arr)
		{
			ptrptr[i++] = lnk;
		}
	}
	int run()
	{
		do
			switch ([this]()->int {
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
				for (size_t i = 0; i < current; i++)
					std::cout << ptrptr[i].c_str() << '\n';
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN
					| FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE);
				std::cout << ptrptr[current].c_str();
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN
					| FOREGROUND_RED | FOREGROUND_INTENSITY);
				std::cout << '\n';
				for (size_t i = current + 1; i < _size; i++)
					std::cout << ptrptr[i].c_str() << '\n';
				return _getch(); }())
			{
			case Enter:
				_funcs[current]();
				break;
			case 72:
				current = --current < 0 ? _size - 1 : current;
				break;
			case 80:
				current = ++current >= _size ? 0 : current;
				break;
			case Escape:
				menurunning = false;
				break;
			}
				while (menurunning);
				return 0;
	}
protected:
	bool menurunning;
	short current, _size;
	std::string ptrptr[menusize];
	std::function<void()> _funcs[menusize];
};