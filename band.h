#ifndef BAND_H_H_H
#define BAND_H_H_H

#include <iostream>
#include <string>
#include <list>
#include "gpio.h"
#include "device.h"

using namespace std;

enum rf_path_type {
	RF_PATH_RX0,
	RF_PATH_RX1,
	RF_PATH_TX0,
	RF_PATH_NUM,
	RF_PATH_INVALID = RF_PATH_NUM,
};

class Band{
public:
	Band(string name);
	string getName(void);
	bool isCompatible(string name);
	enum rf_path_type pathType(string name);
	void setWtrPort(int path, string port);
	string getWtrPort(int path);
	void addDeviceCfg(int path, DeviceCfg dev);
	list<DeviceCfg> getDeviceCfg(int path);
	void addGpioCfg(int path, GpioCfg gpio);
	list<GpioCfg> getGpioCfg(int path);
private:
	/* band name read from mipi table */
	string mName;
	string wtr_port[RF_PATH_NUM];
	list<DeviceCfg> dev_cfg[RF_PATH_NUM];
	list<GpioCfg> sig_cfg[RF_PATH_NUM];
};

#endif


