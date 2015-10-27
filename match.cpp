#include <cstring>
#ifdef __cplusplus
extern "C" {
#endif
	#include <regex.h>
	#include <errno.h>
	#include <error.h>
#ifdef __cplusplus
}
#endif
#include <cassert>

using namespace std;

int isMatch(const char *line, const char *reg)
{
	int ret;
	regex_t preg;
	regmatch_t pmatch[1];
	ret = regcomp(&preg, reg, REG_EXTENDED);
	if(ret)
		error_at_line(1, errno, __FILE__, __LINE__, "reg compile error.");
	ret = regexec(&preg, line, 1, pmatch, 0);
	regfree(&preg);
	if(ret == 0)
		return 1;
	else
		return 0;
}
void getMatch(const char *line, const char *reg, char *match)
{
	int ret;
	regex_t preg;
	regmatch_t pmatch[1];
	ret = regcomp(&preg, reg, REG_EXTENDED);
	if(ret)
		error_at_line(1, errno, __FILE__, __LINE__, "reg compile error.");
	ret = regexec(&preg, line, 1, pmatch, 0);
	assert(ret == 0);
	if(pmatch[0].rm_so == -1)
		match[0] = '\0';
	else {
		memcpy(match, line + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
		match[pmatch[0].rm_eo - pmatch[0].rm_so] = '\0';
	}
	regfree(&preg);
}

int isMatchSigCfg(const char *line)
{
	char pattern[] = "rfc_sig_info_type rf_card_.*_[rt]x[01]_[gsmwcdalte]+_[bgc]+[0-9]*_*[abc]*_sig_cfg";
	int ret = isMatch(line, pattern);
	if(ret) {
		char exclude_pattern[] = "init";
		if(isMatch(line, exclude_pattern))
			ret = 0;
	}
	return ret;
}

void getMatchSigCfg(const char *line, char *match)
{
	char pattern[] = "rf_card_.*_[rt]x[01]_[gsmwcdalte]+_[bgc]+[0-9]*_*[abc]*_sig_cfg";
	getMatch(line, pattern, match);
}

void getMatchDevInfo(const char *line, char *match)
{
	char pattern[] = "rf_card_.*_[rt]x[01]_[gsmwcdalte]+_[bgc]+[0-9]*_*[abc]*_device_info";
	getMatch(line, pattern, match);
}

int isMatchDevInfo(const char *line)
{
	char pattern[] = "rfc_device_info_type rf_card_.*_[rt]x[01]_[gsmwcdalte]+_[bgc]+[0-9]*_*[abc]*_device_info";
	int ret = isMatch(line, pattern);
	if(ret) {
		char exclude_pattern[] = "init";
		if(isMatch(line, exclude_pattern))
			ret = 0;
	}
	return ret;
}

