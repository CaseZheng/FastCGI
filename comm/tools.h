#ifndef _TOOLS_H_
#define _TOOLS_H_ 

#undef signal_set

#include <sstream>

using namespace std;

class tools 
{
public:
    static string GetHostNameIp();
    static string GetFileNameFromFilePath(const string &strFilePath);
    static string GetRandomString();
    static string GetProcessName();
    static string PrintHex(const vector<char>& buff);
    static int GetTimeStamps();

    template<typename T>
    static void insert_number(T number, vector<char> &buf)
    {
        char tmp[sizeof(T)] = {0};
        memcpy(tmp, (char*)&number, sizeof(T));
        for(int i=0; i<sizeof(T); ++i)
        {
            buf.push_back(tmp[i]);
        }
    }
};

#endif
