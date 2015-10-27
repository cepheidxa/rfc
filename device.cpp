#include <iostream>
#include <string>
#include <cassert>
#include "device.h"

using namespace std;

Device::Device(string name, unsigned long pid, unsigned long mid, unsigned long default_usid, unsigned long assigned_usid, int instance)
{
	assert(!name.empty());
	mName = name;
	mMid = mid;
	mPid = pid;
	mDefaultUsid = default_usid;
	mAssignedUsid = assigned_usid;
	mModuleInstance = instance;
	mBusComm = -1;

	if(mName.find("wtr") == 0 || mName.find("WTR") == 0)
		mType = RFDEVICE_WTR;
	else if(mName.find("asm") == 0 || mName.find("ASM") == 0)
		mType = RFDEVICE_ASM;
	else if(mName.find("pa") == 0 || mName.find("PA") == 0)
		mType = RFDEVICE_PA;
	else if(mName.find("qfe") == 0 || mName.find("QFE") == 0)
		mType = RFDEVICE_PAPM_TUNER;
	else
		mType = RFDEVICE_INVALID;

	assert(mType != RFDEVICE_INVALID);
}

string Device::getName(void)
{
	return mName;
}

void Device::setMid(unsigned long mid)
{
	assert(mid <= 0xFFFF);
	mMid = mid;
}

unsigned long Device::getMid(void)
{
	return mMid;
}

void Device::setPid(unsigned long pid)
{
	assert(pid <= 0xFFFF);
	mPid = pid;
}

unsigned long Device::getPid(void)
{
	return mPid;
}

void Device::setDefaultUsid(unsigned long usid)
{
	assert(usid <= 0xF);
	mDefaultUsid = usid;
}

unsigned long Device::getDefaultUsid(void)
{
return mDefaultUsid;
}

void Device::setAssignedUsid(unsigned long usid)
{
	assert(usid <= 0xF);
	mAssignedUsid = usid;
}

unsigned long Device::getAssignedUsid(void)
{
	return mAssignedUsid;
}

void Device::setModuleInstance(int instance)
{
	assert(instance >= 0);
	mModuleInstance = instance;
}

int Device::getModuleInstance(void)
{
	return mModuleInstance;
}

enum rfdevice_type Device::getType(void)
{
	return mType;
}


DeviceCfg::DeviceCfg(string name, int port, int sec_port, int reg)
{
	mName = name;
	mSecPort = sec_port;
	mPort = port;
	mReg = reg;
}

string DeviceCfg::getName(void)
{
	return mName;
}

void DeviceCfg::setSecPort(int secport)
{
	mSecPort = secport;
};

int DeviceCfg::getSecPort(void)
{
	return mSecPort;
}

void DeviceCfg::setPort(int port)
{
	mPort = port;
}

int DeviceCfg::getPort(void)
{
	return mPort;
}

void DeviceCfg::setReg(int reg)
{
	mReg = reg;
}

int DeviceCfg::getReg(void)
{
	return mReg;
}

enum rfdevice_type DeviceCfg::getType(void)
{
	enum rfdevice_type type;
	if(mName.find("wtr") == 0 || mName.find("WTR") == 0)
		type = RFDEVICE_WTR;
	else if(mName.find("asm") == 0 || mName.find("ASM") == 0)
		type = RFDEVICE_ASM;
	else if(mName.find("pa") == 0 || mName.find("PA") == 0)
		type = RFDEVICE_PA;
	else if(mName.find("qfe") == 0 || mName.find("QFE") == 0)
		type = RFDEVICE_PAPM_TUNER;
	else
		type = RFDEVICE_INVALID;

	assert(type != RFDEVICE_INVALID);

	return type;
}
