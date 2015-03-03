#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <iostream>
#include <string>
#define DISK_SIZE 104857600 //100 MBs
#define DISK_FILE "VirtualDisk.disk"
#define NUMBER_INODES 1024
#define INODE_SIZE 10
#define BLOCK_SIZE 2048
#define FILE_LENGTH 16 // maximum file and directory name length
#define BLOCK_HEADER 24 //FILE LENGTH + BTYE COUNTER + LINK COUNTER
//int *inode_counter = 0;

extern std::string currDir;
extern int currDirInode;
extern int openFiles [1024][3];// an array of the inodes of files that are open with the offsets and flag

int init();
int createInode(char type, std::string name, int flag = 0);
int createBlock(std::string name);
int writeToBlock(int inodeToWrite, char type, int numToWrite, std::string strToWrite);
int setCurrDir(int inode, std::string dir);
int openFile(std::string fname, int flag);
int addOpenFile(int inode, int flag);
int checkIfFileOpen(int inode);
int writeToFile(int fd, std::string str);
int readFromFile(int fd, int size);
int seekFile(int fd, int offset);
int closeFile(int fd);
std::string readFromBlock(int fd, int inodeToWrite, int offset, int size, bool readAll = false);