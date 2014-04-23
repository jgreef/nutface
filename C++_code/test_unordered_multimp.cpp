// This file is a test of the C++ unordered multimap

#include <iostream>
#include <string>
#include <unordered_map>

int main()
{
	std::unordered_multimap<unsigned int, unsigned int> string_map;

	string_map.insert(std::make_pair<unsigned int, unsigned int>(234,567));
	string_map.insert(std::make_pair<unsigned int, unsigned int>(234,789));
	string_map.insert(std::make_pair<unsigned int, unsigned int>(127,203));

	auto range = string_map.equal_range(127);

	for (auto i = range.first; i != range.second; i++)
	{
    	std::cout << i->first << ": " << i->second << std::endl;
    }

    // std::cout << sizeof(int) << std::endl;

    return 0;
}