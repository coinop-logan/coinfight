#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <optional>
#include <functional>
#include <boost/random.hpp>
#include "algorithm.h"
#include "common.h"

using namespace std;

void makeSure(bool condition) // hacky test function
{
    cout << (condition ? "PASSED" : "FAILED") << endl;
}

void makeSure(string name, bool condition) // hacky test function
{
    cout << (condition ? "PASSED: " : "FAILED: ") << name << endl;
}

void doThing(int a)
{
    cout << a << endl;
}

enum Cell { Void, Ground };
Cell charToCell(char c)
{
    if (c == '0') return Void;
    else return Ground;
}
class TerrainMap
{
    vector<vector<Cell>> cells;
public:
    TerrainMap(ifstream* infile)
    {
        cells.clear();

        string line;

        getline(*infile, line);

        stringstream dimSS(line);
        int width, height;
        dimSS >> width >> height;

        while (getline(*infile, line))
        {
            vector<Cell> row;

            for (unsigned int i=0; i<line.size(); i++)
            {
                row.push_back(charToCell(line[i]));
            }

            assert(row.size() == width);
            cells.push_back(row);
        }
        
        assert(cells.size() == height);
    }
    int width() { return cells[0].size(); }
    int height() { return cells.size(); }
};

int main()
{
    ifstream infile("processed.map");
    if (infile.fail())
    {
        cout << "failed to open file" << endl;
        return 1;
    }

    TerrainMap map(&infile);

    cout << map.width() << endl;
    
    return 0;
}