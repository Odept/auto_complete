// http://randomtextgenerator.com

#include <iostream>
#include <fstream>
#include <algorithm>


#define MSG(m) std::cout << m << std::endl


int main(int argc, char** argv)
{
	static const char* s_pathOut = "words.txt";

	if(argc != 2)
	{
		MSG("Usage: <bin> <text file path>");
		return 1;
	}

	std::ifstream fin(argv[1]);
	std::ofstream fout(s_pathOut);

	if(!fin.is_open())
	{
		MSG("Failed to open the input file \"" << argv[1] << "\"");
		return 1;
	}
	if(!fout.is_open())
	{
		MSG("Failed to open the output file \"" << s_pathOut << "\"");
		return 1;
	}

	static const unsigned Letters = 'z' - 'a' + 1;
	char ref[Letters + 1];
	unsigned i;
	for(i = 0; i < Letters; i++)
		ref[i] = 'a' + i;
	ref[i] = 0;

	i = 0;
	for(std::string s;;)
	{
		fin >> s;
		if(fin.eof())
			break;

		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		if(*s.rbegin() == '.')
			s.resize(s.length() - 1);
		if(s.find_first_not_of(ref) != std::string::npos)
		{
			MSG("Skip \"" << s << "\"");
			continue;
		}

		fout << s << std::endl;
		i++;
	}

	MSG("Generated \"" << s_pathOut << "\" with " << i << " lowercase words");

	return 0;
}

