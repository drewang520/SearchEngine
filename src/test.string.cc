#include <string>
#include <iostream>

int main()
{
    std::string s("hello\\r");
    std::cout << "s = " << s << "\n";
    std::cout << "s.back- 1 = " << s.back() << "\n";
    std::cout << "s.back- 2 = " << s.back() << "\n";
}
