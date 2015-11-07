#ifndef SHADER_H_
#define SHADER_H_
#include <string>
using namespace std;

class Shader
    {
    public:
    Shader();
    ~Shader();
    
    const char * load(string );  
    bool compile(); 
    bool output;
    string hold;
    int length;
    private:
};

#endif
