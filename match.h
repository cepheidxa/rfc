#ifndef STRMATCH_H_H_H
#define STRMATCH_H_H_H

int isMatch(const char *line, const char *reg);
void getMatch(const char *line, const char *reg, char *match);
int isMatchSigCfg(const char *line);
void getMatchSigCfg(const char *line, char *match);
void getMatchDevInfo(const char *line, char *match);
int isMatchDevInfo(const char *line);

#endif
