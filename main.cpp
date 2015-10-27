#include <iostream>
#include <cstdlib>
#include "rfc.h"

using namespace std;

/*
 * Usage: main qualcomm_file_name  generated_file_name
 * */

int main(int argc, char *argv[])
{
	if(argc != 3) {
		cerr<<"Usage: main qualcomm_file_name generated_file_name"<<endl;
		exit(-1);
	}

	Rfc rfc;
	rfc.parseConfigFile("config.txt");
	//rfc.dumpConfig();
	if(rfc.checkQualcommCode(argv[1]) != -1)
		rfc.codeGenerate(argv[1],argv[2]);
}
