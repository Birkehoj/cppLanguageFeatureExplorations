#include<iostream>

template<typename T, typename... Ts>
void printVarious(T t, Ts... ts)
{
	using std::cout;
	cout << "Printing various arguments\n";
	cout << t;
	((cout << ", " << ts), ...);
	cout << std::endl;
}

int main()
{
	printVarious(1, 2.43, 'w', "it works!!!");
	std::cin.ignore();
	return 0;
}