#include <cstddef>
#include <iostream>
#include <string>

int main()
{
    std::string chinese = "你好中国";
    int len = 1;
    for (size_t i = 0; i < chinese.size();)
    {
        unsigned char c = chinese[i];
        if (c >= 0xF0) len = 4;
        else if (c >= 0xE0) len = 3;
        else if (c >= 0xC0) len = 2;

        std::string ch = chinese.substr(i, len);
        std::cout << "字符: " << ch << "\n";
        i += len;
    }
    return 0;
}
