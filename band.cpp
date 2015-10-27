#include <iostream>
#include <map>
#include <string>
#include <list>
#include <cassert>
#include <cstdlib>
#include "band.h"

using namespace std;

Band::Band(string name)
{
	mName = name;
	assert(mName.length());
}

string Band::getName(void)
{
	return mName;
}

bool Band::isCompatible(string pathCfgName)
{
	string tmp;
	if(mName.find_first_of("gG") != string::npos) //gsm
		tmp = "gsm_g";
	else if(mName.find_first_of("lL") != string::npos) //lte
		tmp = "lte_b";
	else if(mName.find_first_of("wW") != string::npos) //wcdma
		tmp = "wcdma_b";
	else if(mName.find_first_of("cC") != string::npos) //cdma
		tmp = "cdma_bc";

	int num_pos = mName.find_first_of("123456789");
	assert(num_pos != string::npos);

	int band = strtol(mName.substr(num_pos).c_str(), NULL, 10);
	tmp += band;

	//check split band name
	int split_pos = mName.find_first_not_of("0123456789",num_pos);
	switch(mName[split_pos]) {
		case 'b':
		case 'B':
		case 'c':
		case 'C':
			tmp += "_";
			tmp += tolower(mName[split_pos]);
			if(pathCfgName.find(tmp) != string::npos)
				return true;
			break;
		default:
			if(pathCfgName.find(tmp) != string::npos) {
				if(pathCfgName.find(tmp + "_b") == string::npos && pathCfgName.find(tmp + "_c") == string::npos)
					return true;
			}
			break;
	}
	return false;
}

enum rf_path_type Band::pathType(string name)
{
	if(name.find("rx0") != string::npos)
		return RF_PATH_RX0;
	else if(name.find("rx1") != string::npos)
		return RF_PATH_RX1;
	else if(name.find("tx") != string::npos)
		return RF_PATH_TX0;
	else
		return RF_PATH_INVALID;
}

void Band::setWtrPort(int path, string port)
{
	assert(path < RF_PATH_INVALID);
	wtr_port[path] = port;
}

string Band::getWtrPort(int path)
{
	assert(path < RF_PATH_INVALID);
	return wtr_port[path];
}

void Band::addDeviceCfg(int path, DeviceCfg dev)
{
	assert(path < RF_PATH_INVALID);
	dev_cfg[path].push_back(dev);
}

list<DeviceCfg> Band::getDeviceCfg(int path)
{
	assert(path < RF_PATH_INVALID);
	return dev_cfg[path];
}

void Band::addGpioCfg(int path, GpioCfg gpio)
{
	assert(path < RF_PATH_INVALID);
	sig_cfg[path].push_back(gpio);
}

list<GpioCfg> Band::getGpioCfg(int path)
{
	assert(path < RF_PATH_INVALID);
	return sig_cfg[path];
}
