#include <iostream>
#include <cstdlib>
#include <cassert>
#ifdef __cplusplus
extern "C" {
#endif
	#include <getopt.h>
#ifdef __cplusplus
}
#endif
#include "rfc.h"
#include "config.h"

using namespace std;

/*
 * Usage: main qualcomm_file_name  generated_file_name
 * */



static void usage(void)
{
	cerr<<"Usage: main qualcomm_file_name generated_file_name"<<endl;
	cerr<<"Please report any bugs to "<<PACKAGE_BUGREPORT<<endl;
	exit(-1);
}

struct option long_options[] = {
	{"version", 0, NULL, 'v'},
	{"input", 1, NULL, 'i'},
	{"output", 1, NULL, 'o'},
	{0, 0, 0, 0},
};


int main(int argc, char *argv[])
{
	string infile;
	string outfile;
	int c;
	while((c = getopt_long(argc, argv, "vi:o:", long_options, NULL)) != -1) {
		switch(c) {
			case 'v':
				cout<<VERSION<<endl;
				exit(0);
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			default:
				usage();
		}
	}

	if(infile.length() == 0 || outfile.length() == 0)
		usage();

	Rfc rfc;
	rfc.parseConfigFile("config.txt");
	//rfc.dumpConfig();
	if(rfc.checkQualcommCode(infile) != -1)
		rfc.codeGenerate(infile, outfile);
	return 0;
}
