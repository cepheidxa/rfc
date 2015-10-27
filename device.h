#ifndef DEVICE_H_H_H
#define DEVICE_H_H_H
#include <iostream>
#include <string>

using namespace std;

enum rfdevice_type {
	RFDEVICE_WTR,
	RFDEVICE_ASM,
	RFDEVICE_PA,
	RFDEVICE_PAPM_TUNER,
	RFDEVICE_HDET,
	RFDEVICE_INVALID,
};

class Device {
public:
	Device(string name, unsigned long pid = 0, unsigned long mid = 0, unsigned long default_usid = 0, unsigned long assigned_usid = 0, int instance = -1);
	string getName(void);
	void setMid(unsigned long mid);
	unsigned long getMid(void);
	void setPid(unsigned long pid);
	unsigned long getPid(void);
	void setDefaultUsid(unsigned long usid);
	unsigned long getDefaultUsid(void);
	void setAssignedUsid(unsigned long usid);
	unsigned long getAssignedUsid(void);
	void setModuleInstance(int instance);
	int getModuleInstance(void);
	enum rfdevice_type getType(void);
private:
	string mName;
	unsigned long mMid;
	unsigned long mPid;
	unsigned long mDefaultUsid;
	unsigned long mAssignedUsid;
	int mModuleInstance;
	int mBusComm;
	enum rfdevice_type mType;
};


class DeviceCfg {
public:
	DeviceCfg(string name, int port = -1, int sec_port = -1, int reg = -1);
	string getName(void);
	void setSecPort(int secport);
	int getSecPort(void);
	void setPort(int port);
	int getPort(void);
	void setReg(int reg);
	int getReg(void);
	enum rfdevice_type getType(void);
private:
	string mName;
	int mSecPort;
	int mPort;
	int mReg;
};

#endif
