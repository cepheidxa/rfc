#ifndef RFC_H_H_H
#define RFC_H_H_H
#include <iostream>
#include <string>
#include "device.h"
#include "gpio.h"
#include "band.h"

using namespace std;

enum config_type {
	CONFIG_GPIO,
	CONFIG_DEVICE,
	CONFIG_BAND,
	CONFIG_INVALID,
};

class Rfc {
public:
	Rfc();
	~Rfc();
	void parseConfigFile(string filename);
	/* return 0 if the code can be dealed with. else return -1 */
	int checkQualcommCode(string filename);
	void codeGenerate(string infilename, string outfilename);
	void dumpConfig(void);
private:
	/* read all filename's printable character \n or \t except ' ' to configData, */
	void readConfigFileData(string filename);
	/* covert all character to lower case for comaring. if there is no value, take it as -1 */
	int configReadLine(char *line);
	enum config_type configType(char *line);
	void parseSigCfg(ifstream &in, ostringstream &out, char *path);
	list<GpioCfg> getSigInfo(char *path);
	void parseDevCfg(ifstream &in, ostringstream &out, char *path);
	list<DeviceCfg> getDevInfo(char *path);
	string getWtrPort(char *path);

	char *configData;
	map<int, string> gpioMap;
	map<string, Device> devMap;
	list<string> devInBandCfg;
	list<Band> bandList;
};

#endif
