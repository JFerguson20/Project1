#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#define DISK_SIZE 104857600 //100 MBs
#define DISK_FILE "VirtualDisk.disk"
#define NUMBER_INODES 2028
#define INODE_SIZE 14 //1 TYPE (char) + link count(int) + block1(int) + block2+(int)
#define BLOCK_SIZE 32768 //32KB
#define FILE_LENGTH 16 // maximum file and directory name length
#define BLOCK_HEADER 24 //FILE LENGTH + BTYE COUNTER + parent dir inode
//int *inode_counter = 0;

extern std::string currDir;
extern int currDirInode;
extern int openFiles [1024][3];// an array of the inodes of files that are open with the offsets and flag

int init();
int createInode(char type, std::string name, int src = 0, int flag = 0);
int createBlock(std::string name);
int writeToBlock(int inodeToWrite, char type, int numToWrite, std::string strToWrite, int offset = 0);
int setCurrDir(int inode, std::string dir);
int openFile(std::string fname, int flag);
int addOpenFile(int inode, int flag);
int checkIfFileOpen(int inode);
int writeToFile(int fd, std::string str);
int readFromFile(int fd, int size);
int seekFile(int fd, int offset);
int closeFile(int fd);
int createDir(int inode, std::string dirname);
int removeDir(std::string dirname);
int changeDir(int inode, std::string dirname);
int ls();
int linkFiles(int srcInode, int destInode, std::string src, std::string dest);
int unlinkFiles(int inode, std::string dest);
int stats(std::string filename);
int catFile(std::string filename);
int printTree(int dirInode, int depth);
int importFile(std::string src, std::string dest);
int exportFile(std::string src, std::string dest);
int copy(int srcInode, int destInode, std::string src, std::string dest);
//helper methods
std::string readFromBlock(int fd, int inodeToWrite, int offset, int size, bool readAll = false);
int createSecondBlock();
int numOfBlocks(int inodeIndex);
void incrementLink(int inode);
void decrementLink(int inode);
int eraseFileFromDir(int blockIndex, int deleteIndex);
void eraseInode(int inode);
bool isEmpty(int inode);
int findDirInode(std::string input);
int findDir(int inode, std::string dirname);
std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true);
