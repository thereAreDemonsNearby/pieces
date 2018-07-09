#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <cinttypes>
#include "UnicodeConvert.h"

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <numperline>\n";
		std::exit(0);
	}
	const std::string content_u8{std::istreambuf_iterator<char>(std::cin),
			std::istreambuf_iterator<char>()};
	std::cout << "Confirm: " << content_u8 << std::endl;
	std::u16string content;
	slt::Utf8ToUtf16(content_u8.begin(), content_u8.end(),
					 std::back_inserter(content));
	
	std::size_t ind = 0;
	std::vector<std::string> symbols;
	while (ind < content.size()) {
		if (content[ind] == '/') {
			if (ind == content.size() - 1) {
				std::cerr << "format error" << std::endl;
				std::exit(1);
			}
			
			if (content[ind + 1] == '/') {
				// line comment
				ind += 2;
				while (ind < content.size() && content[ind] != '\n') {
					ind++;
				}
				if (ind < content.size())
					++ind;
				
			} else if (content[ind + 1] == '*') {
				// block comment
				ind += 2;
				// char16_t commentend[3];
				// commentend[0] = '*';
				// commentend[1] = '/';
				// commentend[2] = 0;
				while (ind < content.size()
					   && content.substr(ind, 2) != u"*/") {
					++ind;
				}
				if (ind < content.size())
					ind += 2;
				else {
					std::cerr << "format error" << std::endl;
					std::exit(2);
				}
			} else {
				std::cerr << "format error" << std::endl;
				std::exit(3);
			}
		} else if (std::isblank((char)content[ind])) {
			while (ind < content.size() && std::isblank(content[ind])) {
				++ind;
			}
		
		} else if (content[ind] == ',') {
			ind++;
		} else if (content[ind] < 128
				   && std::isalnum(content[ind])) {
			std::string lexeme;
			while (content[ind] < 128 && std::isalnum(content[ind])) {
				lexeme.push_back(std::tolower(content[ind++]));
			}

			symbols.push_back(lexeme);
		} else {
			std::cerr << "format error" << std::endl;
			std::cerr << (int)content[ind] << std::endl;		   
			std::exit(4);
		}		
	}

	std::cout << "\nResult:\n";
	int cnt = 0;
	int perline = std::stoi(argv[1]);
	bool newLineTerminated = false;
	for (const auto& str: symbols) {
		cnt++;
		std::cout << "\"" << str << "\",";
		if (cnt % perline == 0) {
			std::cout << "\n";
			newLineTerminated = true;
		} else {
			std::cout << " ";
			newLineTerminated = false;
		}
	}

	if (!newLineTerminated)
		std::cout << '\n';
}
