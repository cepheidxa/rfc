#include <iostream>
#include <map>
#include <cassert>
#include <cstdlib>
#include "gpio.h"

using namespace std;

GpioCfg::GpioCfg(int gpio, int level)
{
	assert(gpio >= 0);
	assert(level == 1 || level == 0);
	mGpio = gpio;
	mLevel = level;

}

int GpioCfg::getNum(void)
{
	return mGpio;
}

int GpioCfg::getLevel(void)
{
	return mLevel;
}
