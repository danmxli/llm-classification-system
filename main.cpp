#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "trie.hpp"

using namespace std;

vector<string> parseComma(string classifications)
{
	istringstream stream(classifications);
	string singleClassification;

	vector<string> classificationVector;
	while (getline(stream, singleClassification, ','))
	{
		classificationVector.push_back(singleClassification);
	}

	return classificationVector;
}

int main()
{

	Trie trie;

	string command;

	while (cin >> command)
	{
		if (command == "LOAD")
		{
			string filename;
			cin >> filename;

			ifstream inputFile(filename);
			string classifications;

			while (getline(inputFile, classifications))
			{
				trie.insert(parseComma(classifications));
			}
			cout << "success" << endl;
		}
		else if (command == "INSERT")
		{
			string classifications;
			cin.ignore();
			getline(cin, classifications);

			vector<string> classificationVector = parseComma(classifications);

			Trie::ReturnCodes res = trie.insert(classificationVector);
			if (res == Trie::SUCCESS)
			{
				cout << "success" << endl;
			}
			else if (res == Trie::FAILURE)
			{
				cout << "failure" << endl;
			}
		}
		else if (command == "CLASSIFY")
		{
			string input;
			cin.ignore();
			getline(cin, input);
			trie.classify(input);
		}
		else if (command == "ERASE")
		{
			string classifications;
			cin.ignore();
			getline(cin, classifications);

			vector<string> classificationVector = parseComma(classifications);

			trie.erase(classificationVector);
		}
		else if (command == "PRINT")
		{
			trie.print(trie.getRoot());
		}
		else if (command == "EMPTY")
		{
			trie.printEmpty();
		}
		else if (command == "CLEAR")
		{
			trie.clear();
		}
		else if (command == "SIZE")
		{
			trie.printSize();
		}
		else if (command == "EXIT")
		{
			break;
		}
	}

	return 0;
}
