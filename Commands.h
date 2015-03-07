#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <vector>
#include "Disk.h"
#include <stdlib.h>
#include <sstream>
#include <fstream>
int mkfs(std::vector<std::string> params);
int open(std::vector<std::string> params);
int read(std::vector<std::string> params);
int write(std::vector<std::string> params, std::string str);
int seek(std::vector<std::string> params);
int close(std::vector<std::string> params);
//in progress
int mkdir(std::vector<std::string> params);
int rmdir(std::vector<std::string> params);
int cd(std::vector<std::string> params);
int ls(std::vector<std::string> params);
//add rest of commands

