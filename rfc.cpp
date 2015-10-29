#include <iostream>
#include <string>
#include <cassert>
#include <list>
#include <map>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <cstring>
#include "gpio.h"
#include "device.h"
#include "band.h"
#include "rfc.h"
#include "match.h"

using namespace std;

Rfc::Rfc()
{
	configData = NULL;
}

Rfc::~Rfc()
{
	if(configData)
		delete[] configData;
}

void Rfc::parseConfigFile(string filename)
{
	readConfigFileData(filename);
	char *line = new char[LINE_MAX_LEN];
	assert(line);
	while(configReadLine(line) > 0) {
		rescan:
		if(configType(line) == CONFIG_GPIO) { //pase gpio
			while (configReadLine(line) > 0) {
				if((configType(line) != CONFIG_GPIO) &&(configType(line) != CONFIG_INVALID))
					goto rescan;
				int gpio;
				char name[STRING_LEN_MAX];
				sscanf(line, "%d\t%s", &gpio, name);
				char *pch = name;
				while(*pch) {
					*pch = toupper(*pch);
					pch++;
				}
				assert(gpio >= 0);
				assert(name[0] != '\0');
				
				gpioMap.insert(pair<int, string>(gpio, name));
				
			}
		} else if(configType(line) == CONFIG_DEVICE){ //parse device
			while (configReadLine(line) > 0) {
				if((configType(line) != CONFIG_DEVICE) &&(configType(line) != CONFIG_INVALID))
					goto rescan;
				char name[STRING_LEN_MAX];
				unsigned long mid, pid, default_usid, assigned_usid, device_module_type_instance;
				sscanf(line, "%s\t%lx\t%lx\t%lx\t%lx\t%ld", name, &pid, &mid, &default_usid, &assigned_usid, &device_module_type_instance);
				Device dev(name, pid, mid, default_usid, assigned_usid, device_module_type_instance);
				assert(device_module_type_instance >= 0);
				devMap.insert(pair<string, Device>(name, dev));
			}
		} else if(configType(line) == CONFIG_BAND){ //parse band
			//"band\twtr_ports\tasm-sky13455\t-1\tasm-sky13526\t-1\tpa-sky77631\t-1\tpa-acpm-9307\t-1\tqfe2101\t-1\tqfe2520\t-1\tgpio\t"
			istringstream iss;
			iss.str(line);
			string tmp;
			iss>>tmp;
			assert(tmp.compare("band") == 0);
			iss>>tmp;
			assert(strstr(tmp.c_str(),"wtr") && strstr(tmp.c_str(), "port"));
			while(iss>>tmp) {
				if(tmp.length() && tmp.compare("gpio") && tmp.compare("-1"))
					devInBandCfg.push_back(tmp);
			}
			int device_count = devInBandCfg.size();

			/* line example, if there is no value in config.txt, take it sa -1 or NA
			 *
			 *g850    rx0_port  REG value    PortNum  Sec_PortNum  PortNum  REG value    PortNum  REG value    PortNum  -1
			 *-1      prx_lb3   0x9          6        -1           -1       -1          -1        -1           -1       -1
			 *-1      tx_port   REG value    PortNum  Sec_PortNum  PortNum  REG value    PortNum  REG value    PortNum  -1
			 *-1      prx_lb3   0xa          13           9        -1       -1           -1       -1           -1       gpio_84-1/gpio_92-0
			 *g900    rx0_port  REG value    PortNum  Sec_PortNum  PortNum  REG value    PortNum  REG value    PortNum  -1
			 *-1      prx_lb2   0x3          ............................
			 */
			 Band *band = NULL;
			while(configReadLine(line) > 0) {
				istringstream iss_head;
				iss_head.str(line);
				assert(configReadLine(line) > 0);
				istringstream iss_val;
				iss_val.str(line);

				string tmp, tmp_val;
				iss_head>>tmp;
				iss_val>>tmp_val;
				string band_name;
				if(tmp.compare("-1")) {
					band_name = tmp;
					if(band) {
						bandList.push_back(*band);
						delete band;
						band = NULL;
					}
					band = new Band(band_name);
					assert(band);
				}
				//check wtr-port
				iss_head>>tmp;
				iss_val>>tmp_val;
				enum rf_path_type path = band->pathType(tmp);
				assert(path != RF_PATH_INVALID);
				band->setWtrPort(path, tmp_val);

				//parse device
				for(list<string>::iterator it = devInBandCfg.begin(); it != devInBandCfg.end(); ++it) {
					DeviceCfg dev(*it);
					if(dev.getType() == RFDEVICE_ASM) {
						iss_head>>tmp>>tmp; //reg port
						iss_val>>tmp_val;
						if(tmp_val.find_first_of("0123456789") != string::npos)
							dev.setReg(strtol(tmp_val.c_str(), NULL, 16));
						else
							dev.setReg(-1);

						iss_val>>tmp_val;
						if(tmp_val.find_first_of("0123456789") != string::npos)
							dev.setPort(strtol(tmp_val.c_str(), NULL, 10));
						else
							dev.setPort(-1);
					} else if(dev.getType() == RFDEVICE_PA) {
						iss_head>>tmp>>tmp; //sec_port port
						iss_val>>tmp_val;
						if(tmp_val.find_first_of("0123456789") != string::npos)
							dev.setSecPort(strtol(tmp_val.c_str(), NULL, 10));
						else
							dev.setSecPort(-1);

						iss_val>>tmp_val;
						if(tmp_val.find_first_of("0123456789") != string::npos)
							dev.setPort(strtol(tmp_val.c_str(), NULL, 10));
						else
							dev.setPort(-1);
					} else if(dev.getType() == RFDEVICE_PAPM_TUNER) {
						iss_head>>tmp>>tmp;
						iss_val>>tmp_val>>tmp_val;
					}
					else {
						cerr<<"device: "<<*it<<", can't get device type(asm, pa papam tuner)."<<endl;
						exit(-1);
					}
					if(dev.getReg() != -1 || dev.getSecPort() != -1 || dev.getPort() != -1)
						 band->addDeviceCfg(path, dev);
				}

				//parse gpio
				if(iss_val>>tmp_val) {
					int pos = 0;
					while((pos = tmp_val.find("gpio", pos)) != string::npos) {
						pos += 4;
						int value[2];
						istringstream iss_gpio;
						iss_gpio.str(tmp_val.substr(pos));
						for(int i = 0; i < 2; i++) {
							char ch = '\0';
							while(!isdigit(ch))
								iss_gpio>>ch;
							iss_gpio.putback(ch);
							iss_gpio>>value[i];
						}
						GpioCfg gpio(value[0], value[1]);
						band->addGpioCfg(path, gpio);
					}
				}
			}
			if(band) {
				bandList.push_back(*band);
				delete band;
				band = NULL;
			}
		}
	}
	delete[] line;
}

void Rfc::readConfigFileData(string filename)
{
	ifstream config(filename.c_str(), ifstream::binary);
	assert(config.is_open());
	//get length of file
	config.seekg(0, config.end);
	int length = config.tellg();
	config.seekg(0, config.beg);
	assert(configData == NULL);
	configData = new char[length + 2];
	assert(configData != NULL);
	config.read(configData, length);
	configData[length] = '\n';
	configData[length + 1] = '\0';
	config.close();

	/* check config format, if it's unicode text, 2 byte each character,
	 * begining with 0xFFFE means unicode text file
	 */
	int unicode = 0;
	if(static_cast<unsigned char>(configData[0]) == 0xFF && static_cast<unsigned char>(configData[1]) == 0xFE) {
		unicode = 1;
	}
	else if(tolower(static_cast<unsigned char>(configData[0])) != 'g' || tolower(static_cast<unsigned char>(configData[1])) !='p') {
		cerr<<"ERROR: config file format is wrong, please save excel file as text, and gpio must be the first configuration."<<endl;
		exit(-1);
	}

	int i = 0;
	int j = 0;
	while(i < length) {
		if(isprint((unsigned char)(configData[i])) || configData[i] == '\n' || configData[i] == '\t')
			if(configData[i] != ' ')
				configData[j++] = configData[i];
		i++;
	}
	configData[j++] = '\n';
	configData[j] = '\0';
}

int Rfc::configReadLine(char *line)
{
	assert(line);
	assert(configData);
	static int pos = 0;
	int i = 0;
	unsigned char ch_pre = 0;
	if(configData[pos] == '\0')
		return 0;
	rescan:
	i = 0;
	ch_pre = '\0';
	line[0] == '\0';
	//skip empty line
	while(configData[pos] == '\n')
		pos++;
	while(configData[pos] && configData[pos] !='\n') {
		unsigned char ch = configData[pos++];
		if(isupper(ch))
			line[i++] = tolower(ch);
		else if(ch == '\t') {
			if(ch_pre == '\t' || ch_pre == '\0') {
				line[i++] = '-';
				line[i++] = '1';
			}
			line[i++] = ch; 
		} else
			line[i++] = ch;
		ch_pre = ch;
	}	
	line[i] = '\0';
	if(line[0] == '\0' && configData[pos] != '\0') //empty line
		goto rescan;
	return i;
}

enum config_type Rfc::configType(char *line)
{
	if(strstr(line, "gpio") && strstr(line, "name"))
		return CONFIG_GPIO;
	else if(strstr(line, "device") && strstr(line, "mid") && strstr(line, "pid"))
		return CONFIG_DEVICE;
	else if(strstr(line, "band") && strstr(line, "port") && strstr(line, "wtr")/* && strstr(line, "asm") && strstr(line, "pa")*/)
		return CONFIG_BAND;
	else
		return CONFIG_INVALID;
}

int Rfc::checkQualcommCode(string filename)
{
	return 0;
}

void Rfc::codeGenerate(string infilename, string outfilename)
{
	ifstream in(infilename.c_str());
	assert(in.is_open());
	
	ostringstream out;
	char *line = new char[LINE_MAX_LEN];
	assert(line);
	while (in.getline(line, LINE_MAX_LEN)) {
		line[strlen(line) + 1] = '\0';
		line[strlen(line)] = '\n';
		if(isMatchSigCfg(line)) {
			char rf_path[STRING_LEN_MAX];
			getMatchSigCfg(line, rf_path);
			out<<line;
			if(strlen(rf_path))
				parseSigCfg(in, out, rf_path);
		}
		else if(isMatchDevInfo(line)) {
			char rf_path[STRING_LEN_MAX];
			getMatchDevInfo(line, rf_path);
			out<<line;
			if(strlen(rf_path))
				parseDevCfg(in, out, rf_path);
		}
		else
			out<<line;
	}
	delete[] line;
	ofstream fout(outfilename.c_str());
	assert(fout.is_open());
	fout<<out.str();
}

void Rfc::dumpConfig(void)
{
	cout.setf(ios::left);
	//gpio name
	cout<<setw(6)<<"gpio"<<"name"<<endl;
	for(map<int, string>::iterator it = gpioMap.begin(); it != gpioMap.end(); ++it)
		cout<<setw(6)<<it->first<<it->second<<endl;

	//device pid mid .....
	cout<<setw(15)<<"DEVICE"<<setw(10)<<"PID"<<setw(10)<<"MID"<<setw(15)<<"DEFAULT_USID"<<setw(15)<<"ASSIGNED_USID"<<"DEVICE_MODULE_TYPE_INSTANCE"<<endl;
	for(map<string, Device>::iterator it = devMap.begin(); it != devMap.end(); ++it) {
		Device *dev = &it->second;
		cout<<setw(15)<<dev->getName()<<"0x"<<setw(8)<<hex<<dev->getPid()<<"0x"<<setw(8)<<hex<<dev->getMid()<<"0x"<<setw(13)<<hex<<dev->getDefaultUsid();
		cout<<"0x"<<setw(13)<<hex<<dev->getAssignedUsid()<<dev->getModuleInstance()<<endl;
	}

	//band ...
	cout<<setw(8)<<"Band"<<setw(10)<<"WTR-Port";
	for(map<string, Device>::iterator it = devMap.begin(); it != devMap.end(); ++it) {
		Device *dev = &it->second;
		cout<<setw(22)<<dev->getName();
	}
	cout<<"gpio"<<endl;
	for(list<Band>::iterator it = bandList.begin(); it != bandList.end(); ++it) {
		for(int path = RF_PATH_RX0; path < RF_PATH_INVALID; ++path) {
			if(it->getWtrPort(path).empty())
				continue;
			if(path == RF_PATH_RX0)
				cout<<setw(8)<<it->getName();
			else
				cout<<setw(8)<<" ";
			switch(path) {
				case RF_PATH_RX0:
					cout<<setw(10)<<"Rx0-port";
					break;
				case RF_PATH_RX1:
					cout<<setw(10)<<"Rx1-port";
					break;
				case RF_PATH_TX0:
					cout<<setw(10)<<"Tx-port";
					break;
				default:
					break;
			}
			for(map<string, Device>::iterator dev_it = devMap.begin(); dev_it != devMap.end(); ++dev_it) {
				if(dev_it->second.getType() == RFDEVICE_PA)
					cout<<setw(13)<<"Sec_PortNum"<<setw(9)<<"PortNum";
				else
					cout<<setw(13)<<"REG_value"<<setw(9)<<"PortNum";
			}
			cout<<"gpio"<<endl;

			//print value
			cout<<setw(8)<<" "<<setw(10)<<it->getWtrPort(path);
			for(map<string, Device>::iterator dev_it = devMap.begin(); dev_it != devMap.end(); ++dev_it) {
				list<DeviceCfg> devCfgList = it->getDeviceCfg(path);
				int devCfg_found = 0;
				list<DeviceCfg>::iterator devCfg_it;
				for(devCfg_it = devCfgList.begin(); devCfg_it != devCfgList.end(); ++devCfg_it) {
					if((devCfg_it->getName()).compare(dev_it->second.getName()) == 0) {
						devCfg_found = 1;
						break;
					}
				}

				if(dev_it->second.getType() == RFDEVICE_PA) {
					if(devCfg_found && devCfg_it->getSecPort() != -1)
						cout<<setw(13)<<devCfg_it->getSecPort();
					else
						cout<<setw(13)<<" ";
					if(devCfg_found && devCfg_it->getPort() != -1)
						cout<<setw(9)<<devCfg_it->getPort();
					else
						cout<<setw(9)<<" ";
				}
				else {
					if(devCfg_found && devCfg_it->getReg() != -1)
						cout<<"0x"<<setw(11)<<hex<<devCfg_it->getReg()<<dec;
					else
						cout<<setw(13)<<" ";
					if(devCfg_found && devCfg_it->getPort() != -1)
						cout<<setw(9)<<devCfg_it->getPort();
					else
						cout<<setw(9)<<" ";
				}
			}
			//print gpio
			list<GpioCfg> gpioCfgList = it->getGpioCfg(path);
			for(list<GpioCfg>::iterator gpioCfg_it = gpioCfgList.begin(); gpioCfg_it != gpioCfgList.end(); ++gpioCfg_it) {
				if(gpioCfg_it != gpioCfgList.begin())
					cout<<"/";
				cout<<"gpio_"<<gpioCfg_it->getNum()<<"-"<<gpioCfg_it->getLevel();
			}
			cout<<endl;
		}
	}
	cout.setf(ios::right);
	cout<<dec;
}

list<GpioCfg> Rfc::getSigInfo(char *path)
{
	list<Band>::iterator it;
	for(it = bandList.begin(); it != bandList.end(); ++it) {
		if(it->isCompatible(path))
			break;
	}

	if(it == bandList.end())
		return list<GpioCfg>();


	if(strstr(path, "rx0"))
		return it->getGpioCfg(RF_PATH_RX0);
	else if(strstr(path, "rx1"))
		return it->getGpioCfg(RF_PATH_RX1);
	else if(strstr(path, "tx0"))
		return it->getGpioCfg(RF_PATH_TX0);
	else {
		cerr<<"pase path "<<path<<" with error"<<endl;
		exit(-1);
	}
}

void Rfc::parseSigCfg(ifstream &in, ostringstream &out, char *path)
{
	char *line = new char[LINE_MAX_LEN];
	assert(line);

	/* {
	 *   RFC_ENCODED_REVISION, 
	 *   {
	 *    { (int)RFC_WTR4905_JAPAN_V2_TX_GTR_TH,   { RFC_CONFIG_ONLY, 0 }, {RFC_LOW, 0 }  },
	 *    { (int)RFC_WTR4905_JAPAN_V2_RF_PATH_SEL_14,   { RFC_LOW, 0 }, {RFC_LOW, 0 }  },
	 *    { (int)RFC_SIG_LIST_END,   { RFC_LOW, 0 }, {RFC_LOW, 0 } }
	 *   },
	 *  };
	 */
	int num = 0;
	while(num <3 && in.getline(line, LINE_MAX_LEN)) {
		line[strlen(line) + 1] = '\0';
		line[strlen(line)] = '\n';
		out<<line;
		num++;
	}
	assert(num == 3);
		
	//get gpio setting
	while (in.getline(line, LINE_MAX_LEN)) {
		line[strlen(line) + 1] = '\0';
		line[strlen(line)] = '\n';
		if(isMatch(line, "RF_PATH_SEL_[0-9]*")) {
			;
		} else if(isMatch(line, "RFC_SIG_LIST_END")) {
			list<GpioCfg> info = getSigInfo(path);
			while(info.size()) {
				if(info.front().getLevel())
					out<<"    { (int)"<<gpioMap.find(info.front().getNum())->second<<",   { RFC_HIGH, 0 }, {RFC_LOW, 0 }  },\n";
				else
					out<<"    { (int)"<<gpioMap.find(info.front().getNum())->second<<",   { RFC_LOW, 0 }, {RFC_LOW, 0 }  },\n";
				info.pop_front();
			}
			out<<line;
			break;

		} else
			out<<line;
	}
	
	num = 0;
	while(num < 2 && in.getline(line, LINE_MAX_LEN)) {
		line[strlen(line) + 1] = '\0';
		line[strlen(line)] = '\n';
		out<<line;
		num++;
	}
	assert(num == 2);
	delete[] line;
}
list<DeviceCfg> Rfc::getDevInfo(char *path)
{
	list<Band>::iterator it;
	for(it = bandList.begin(); it != bandList.end(); ++it) {
		if(it->isCompatible(path))
			break;
	}

	if(it == bandList.end())
		return list<DeviceCfg>();

	if(strstr(path, "rx0"))
		return it->getDeviceCfg(RF_PATH_RX0);
	else if(strstr(path, "rx1"))
		return it->getDeviceCfg(RF_PATH_RX1);
	else if(strstr(path, "tx0"))
		return it->getDeviceCfg(RF_PATH_TX0);
	else {
		cerr<<"pase path "<<path<<" with error"<<endl;
		exit(-1);
	}
}

string Rfc::getWtrPort(char *path)
{
	list<Band>::iterator it;
	for(it = bandList.begin(); it != bandList.end(); ++it) {
		if(it->isCompatible(path))
			break;
	}

	if(it == bandList.end())
		return string();

	if(strstr(path, "rx0"))
		return it->getWtrPort(RF_PATH_RX0);
	else if(strstr(path, "rx1"))
		return it->getWtrPort(RF_PATH_RX1);
	else if(strstr(path, "tx0"))
		return it->getWtrPort(RF_PATH_TX0);
	else {
		cerr<<"pase path "<<path<<" with error"<<endl;
		exit(-1);
	}
}

void Rfc::parseDevCfg(ifstream &in, ostringstream &out, char *path)
{
	char *line = new char[LINE_MAX_LEN];
	assert(line);


	//{
	//  RFC_ENCODED_REVISION, 
	//  RFC_RX_MODEM_CHAIN_1,   /* Modem Chain */
	//  1,   /* NV Container */
	//  RFC_INVALID_PARAM /* Warning: Not Specified */,   /* Antenna */
	//  2,  /* NUM_DEVICES_TO_CONFIGURE */
	//  {

	int num = 0;
	while(in.getline(line, LINE_MAX_LEN)) {
		line[strlen(line) + 1] = '\0';
		line[strlen(line)] = '\n';
		if(strstr(line, "NUM_DEVICES_TO_CONFIGURE"))
			break;
		out<<line;
	}
	in.getline(line, LINE_MAX_LEN);
	line[strlen(line) + 1] = '\0';
	line[strlen(line)] = '\n';
	if(strstr(line, "{")) {
		;
	} else {
		cerr<<"file read error in "<<path<<endl;
		exit(-1);
	}

	int num_devices_to_configure = 0;

	//read qualcomm init devices to devicecode
	list<string> devicesCode;
	while(1) {
		string s = "";
		int in_dev_input = 0;
		while(in.getline(line, LINE_MAX_LEN)) {
			line[strlen(line) + 1] = '\0';
			line[strlen(line)] = '\n';
			if(in_dev_input == 0 && strstr(line, "}"))
				goto read_init_devices_end;
			char *pch = line;
			while(*pch) {
				if(*pch == '{')
					in_dev_input++;
				else if(*pch == '}')
					in_dev_input--;
				pch++;
			}
			s += line;
			if(in_dev_input == 0)
				break;
		}
		devicesCode.push_back(s);
	}
	read_init_devices_end:

	in.getline(line, LINE_MAX_LEN);
	line[strlen(line) + 1] = '\0';
	line[strlen(line)] = '\n';
	assert(strstr(line, "};"));


	list<DeviceCfg> info = getDevInfo(path);
	
	//check wtr port
	assert(devicesCode.size());
	string wtrcode = devicesCode.front();
	devicesCode.pop_front();
	int pos = wtrcode.find("PORT");
	assert(pos != string::npos);
	while(pos) {
		if(!isdigit(static_cast<unsigned char>(wtrcode[pos])))
			pos--;
		else
			break;
	}
	assert(pos > 0);

	string wtrport = getWtrPort(path);
	//if this path is not configed, only wtr is left in dev cfg
	if(wtrport.length() == 0) {
		out<<"  1,  /* NUM_DEVICES_TO_CONFIGURE */\n  {\n";
		out<<wtrcode;
		out<<"    #if 0\n";
		while(devicesCode.size()) {
			out<<devicesCode.front();
			devicesCode.pop_front();
		}
		out<<"    #endif\n";
		out<<"  },\n};\n";
		delete[] line;
		return;
	}
	int wtrport_num_pos = wtrport.find_first_of("0123456789");
	assert(wtrport_num_pos != string::npos);
	wtrcode[pos] = wtrport[wtrport_num_pos];

	ostringstream buf_iss;

	buf_iss<<wtrcode;
	num_devices_to_configure++;

	//check configed device
	#ifdef MIPI_DEVICE_COMPATIBLE
	list<DeviceCfg>::iterator devCfg_it;
	buf_iss.setf(ios::uppercase);
	buf_iss.fill('0');
	//check asm ps
	list<DeviceCfg> asmcfglist;
	list<DeviceCfg> pacfglist;
	for(devCfg_it = info.begin(); devCfg_it != info.end(); devCfg_it++) {
		Device &dev = devMap.find(devCfg_it->getName())->second;
		if(dev.getType() == RFDEVICE_ASM)
			asmcfglist.push_back(*devCfg_it);
		else if(dev.getType() == RFDEVICE_PA)
			pacfglist.push_back(*devCfg_it);
	}
	while(asmcfglist.size()) {
		int com_dev_count = 0;
		DeviceCfg devcfg = asmcfglist.front();
		asmcfglist.pop_front();
		Device &dev = devMap.find(devcfg.getName())->second;
		int instance = devMap.find(devcfg.getName())->second.getModuleInstance();
		num_devices_to_configure++;
		com_dev_count++;
		buf_iss<<"    {\n      RFDEVICE_ASM,\n      GEN_ASM,  /* NAME */\n";
		buf_iss<<"      "<<instance<<",  /* DEVICE_MODULE_TYPE_INSTANCE */\n      0 /*Warning: Not specified*/,  /* PHY_PATH_NUM */\n";
		buf_iss<<"      {\n        0  /* Orig setting:  */,  /* INTF_REV */\n";
		buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
		int found = 1;
		while(found && asmcfglist.size()) {
			found = 0;
			for(list<DeviceCfg>::iterator it = asmcfglist.begin(); it != asmcfglist.end(); ++it) {
				dev = devMap.find(it->getName())->second;
				if(dev.getModuleInstance() == instance) {
					devcfg = *it;;
					asmcfglist.erase(it);
					com_dev_count++;
					found = 1;
					buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
					break;
				}
			}
		}
		assert(com_dev_count<=5);
		while(com_dev_count < 5) {
			buf_iss<<"        0,  /* Array Filler */\n";
			com_dev_count++;
		}
		buf_iss<<"      },\n    },\n";

	}
	while(pacfglist.size()) {
		int com_dev_count = 0;
		DeviceCfg devcfg = pacfglist.front();
		pacfglist.pop_front();
		Device &dev = devMap.find(devcfg.getName())->second;
		int instance = devMap.find(devcfg.getName())->second.getModuleInstance();
		num_devices_to_configure++;
		com_dev_count++;
		buf_iss<<"    {\n      RFDEVICE_PA,\n      GEN_PA,  /* NAME */\n";
		buf_iss<<"      "<<instance<<",  /* DEVICE_MODULE_TYPE_INSTANCE */\n      0 /*Warning: Not specified*/,  /* PHY_PATH_NUM */\n";
		buf_iss<<"      {\n        0  /* Orig setting:  */,  /* INTF_REV */\n";
		if(devcfg.getSecPort() == -1)
			buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
		else
			buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getSecPort()<<" << 7)/*sec_port_num*/ | ("<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
		int found = 1;
		while(found && pacfglist.size()) {
			found = 0;
			for(list<DeviceCfg>::iterator it = pacfglist.begin(); it != pacfglist.end(); ++it) {
				dev = devMap.find(it->getName())->second;
				if(dev.getModuleInstance() == instance) {
					devcfg =	*it;;
					pacfglist.erase(it);
					com_dev_count++;
					found = 1;
					if(devcfg.getSecPort() == -1)
						buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
					else
						buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devcfg.getSecPort()<<" << 7)/*sec_port_num*/ | ("<<devcfg.getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
					break;
				}
			}
		}
		assert(com_dev_count<=5);
		while(com_dev_count < 5) {
			buf_iss<<"        0,  /* Array Filler */\n";
			com_dev_count++;
		}
		buf_iss<<"      },\n    },\n";

	}

	#else
	list<DeviceCfg>::iterator devCfg_it;
	for(devCfg_it = info.begin(); devCfg_it != info.end(); devCfg_it++) {
		Device &dev = devMap.find(devCfg_it->getName())->second;
		buf_iss.setf(ios::uppercase);
		buf_iss.fill('0');
		if(dev.getType() == RFDEVICE_ASM) {
			num_devices_to_configure++;
			buf_iss<<"    {\n      RFDEVICE_ASM,\n      GEN_ASM,  /* NAME */\n";
			buf_iss<<"      "<<dev.getModuleInstance()<<",  /* DEVICE_MODULE_TYPE_INSTANCE */\n      0 /*Warning: Not specified*/,  /* PHY_PATH_NUM */\n";
			buf_iss<<"      {\n        0  /* Orig setting:  */,  /* INTF_REV */\n";
			buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devCfg_it->getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
			buf_iss<<"        0,  /* Array Filler */\n        0,  /* Array Filler */\n        0,  /* Array Filler */\n        0,  /* Array Filler */\n";
			buf_iss<<"      },\n    },\n";
		} else if(dev.getType() == RFDEVICE_PA) {
			num_devices_to_configure++;
			buf_iss<<"    {\n      RFDEVICE_PA,\n      GEN_PA,  /* NAME */\n";
			buf_iss<<"      "<<dev.getModuleInstance()<<",  /* DEVICE_MODULE_TYPE_INSTANCE */\n      0 /*Warning: Not specified*/,  /* PHY_PATH_NUM */\n";
			buf_iss<<"      {\n        0  /* Orig setting:  */,  /* INTF_REV */\n";
			if(devCfg_it->getSecPort() == -1)
				buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devCfg_it->getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
			else
				buf_iss<<"        (0x"<<hex<<setw(4)<<dev.getMid()<<" << 22)/*mfg_id*/ | (0x"<<hex<<dev.getPid()<<" << 14)/*prd_id*/ | ("<<dec<<devCfg_it->getSecPort()<<" << 7)/*sec_port_num*/ | ("<<devCfg_it->getPort()<<")/*port_num*/,  /* PORT_NUM */\n";
				
			buf_iss<<"        0,  /* Array Filler */\n        0,  /* Array Filler */\n        0,  /* Array Filler */\n        0,  /* Array Filler */\n";
			buf_iss<<"      },\n    },\n";
		}
	}
	#endif


	//check hdet qfe(PAPM TUNER)
	for(list<string>::iterator devCode_it = devicesCode.begin(); devCode_it != devicesCode.end(); ++devCode_it) {
		if(devCode_it->find("RFDEVICE_HDET") != string::npos) {
			num_devices_to_configure++;
			buf_iss<<*devCode_it;
		}
		else if(devCode_it->find("RFDEVICE_PAPM") != string::npos || devCode_it->find("RFDEVICE_TUNER") != string::npos) {

				char devname[STRING_LEN_MAX] = {0,};
				if(isMatch(devCode_it->c_str(), "QFE[0-9]+")) {
					getMatch(devCode_it->c_str(), "QFE[0-9]+", devname);
					unsigned char *pch = (unsigned char *)devname;
					while(*pch) {
						*pch = tolower(*pch);
						pch++;
					}
				}
				int dev_found_in_devmap = 0;
				for(map<string, Device>::iterator map_it = devMap.begin(); map_it != devMap.end(); ++map_it) {
					if(map_it->first.find(devname) != string::npos) {
						num_devices_to_configure++;
						buf_iss<<*devCode_it;
						dev_found_in_devmap = 1;
						break;
					}
				}
				if(dev_found_in_devmap == 0) {
					for(list<string>::iterator it = devInBandCfg.begin(); it != devInBandCfg.end(); ++it) {
						if(it->compare(devname) == 0)
						{
							num_devices_to_configure++;
							buf_iss<<*devCode_it;
							dev_found_in_devmap = 1;
						}
					}
				}
			}
	}


	//print device
	out<<"  "<<num_devices_to_configure<<",  /* NUM_DEVICES_TO_CONFIGURE */\n  {\n";
	out<<buf_iss.str();
	out<<"  },\n};\n";
	delete[] line;
}
