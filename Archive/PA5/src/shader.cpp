#ifndef SHADER_CPP_
#define SHADER_CPP_
#include "shader.h"

Shader::Shader()
{

}
Shader::~Shader()
{
	
}

const char * Shader::load(string file) 
{
 

 
    fstream fin;
    string i1,hold;
    hold.clear();
    fin.clear();


    fin.open(file);
       do
	{
	 getline(fin,i1);
	 hold+=i1;
	}
	while (fin.good());
	//fout<<hold;
fin.clear();
    fin.close();

   // vs=new char [i1.size()+1]; 


    return hold.c_str();





 
}

bool Shader:: compile()
{
return false;
}
#endif
