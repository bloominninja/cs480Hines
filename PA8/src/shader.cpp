#ifndef SHADER_CPP_
#define SHADER_CPP_
#include "shader.h"


Shader::Shader()
    {
    
    }
    
Shader::~Shader()
    {
    	hold.clear();
    }
    
const char * Shader::load(string file) 
    {
    // Variable declarations
    fstream fin;
    ofstream fout;
    string i1;
    hold.clear();
    i1.clear();
    fin.clear();
//    fout.open("shaderLoadTest.txt");
    
    fin.open(file);

    

	    getline(fin,i1);
	    hold+=i1;

	while (fin.good())
	{
	    getline(fin,i1);
	    hold+=i1;
	}

	
    fin.close();

    // vs=new char [i1.size()+1]; 
//    fout<<hold.c_str();
//    fout.close();
if(output)
{
fout.open("shaders");
length=hold.length();
fout<<hold;
fout.close();
}
    return hold.c_str();
    }

bool Shader:: compile()
    {
    return false;
    }
#endif
