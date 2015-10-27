#ifndef GPIO_H_H_H
#define GPIO_H_H_H

#include <map>
#include <string>

using namespace std;

class GpioCfg {
public:
	GpioCfg(int gpio, int level);
	int getNum(void);
	int getLevel(void);
private:
	int mGpio;
	int mLevel;
};

#endif
