#include "Disk.h"


std::string currDir ="";
int currDirInode = -1;
//start with all the open files to -1
int openFiles [1024][3];

int init(){
	for(int i =0; i < 1024; i++){
		openFiles[i][0] = -1;
		openFiles[i][1] = 0; //the current offset
		openFiles[i][2] = 0; //file mode
	}
	return 1;
}

//INODE ENTRY
//CHAR - TYPE
//INT - BLOCK ADDRESS
//INT - BLOCK ADDRESS #2 FOR BIGGER FILES

/* type is either d for directory or f for file
*  name is to be passed to createBlock 
*/
int createInode(char type, std::string name, int flag){
	FILE *disk = fopen(DISK_FILE, "r+");
	int blockPtr1;
	int blockPtr2;
	char tempType;
	int *inode_counter = 0;
	int fd = -1;
	//go through each inode until we find a free spot
	for (int i = 0; i < NUMBER_INODES * INODE_SIZE; i += INODE_SIZE){
		fseek(disk, i, SEEK_SET);
		fread(&tempType, sizeof(char), 1, disk);
		//temp type is 0 so it means its empty
		if (!tempType){
			//seek back to the start of the inode before the read
			fseek(disk, i, SEEK_SET);
			//create the first block
			blockPtr1 = createBlock(name);
			blockPtr2 = -1; //starts negative until we need it
			
			//write the type
			fwrite(&type, sizeof(char), 1, disk);
			//write of block1
			fwrite(&blockPtr1, sizeof(int), 1, disk);
			
			//write block2
			fwrite(&blockPtr2, sizeof(int), 1, disk);
			
			//if it was a file add to openFiles
			if (type == 'f'){
				fd = addOpenFile(i, flag);
			}
			//if it was a directory
			else if(type == 'd'){
				
			}
			
			//write this inode (i) to the current directorys block
			//if the current directory has all ready been set
			if(currDirInode >= 0){
				//write this inode (i) to the current directorys block
				
				writeToBlock(currDirInode, type, i, name);
			}else{
				setCurrDir(i, name);
			}

			fclose(disk);
			return fd;
		}
	}
	
	//if we didnt find a free inode
	std::cout << "No free indoes" << std::endl;
	fclose(disk);
	return -1;
}

//BLOCK
//16 BYTES FILE NAME
//INT - NUMBER OF BYTES AFTER THE BLOCK
//INT - LINK COUNT

//creates a block for and returns the start of the blocks offset 
int createBlock(std::string name){
	FILE *disk = fopen(DISK_FILE, "r+");
	char tempBlock;
	
	//The numbe of bytes in the file after the header
	int NumberOfBytes = 0;
	//number of links
	int linkCount = 1;
	//skip the inode table
	for (int i = NUMBER_INODES * INODE_SIZE; i < DISK_SIZE; i += BLOCK_SIZE){
		fseek(disk, i, SEEK_SET);
		fread(&tempBlock, sizeof(char), 1, disk);
		//if a block doesnt already exist here
		if(!tempBlock){
			//go back 1 before the read
			fseek(disk, i, SEEK_SET);
			
			fwrite(&name[0], sizeof(char), FILE_LENGTH, disk);
			fseek(disk, i + FILE_LENGTH, SEEK_SET);
			
			int link_count = 1;
			fwrite(&NumberOfBytes, sizeof(int), 1, disk);
			fwrite(&linkCount, sizeof(int), 1, disk);
			fclose(disk);
			return i;
		}
	}
	
	std::cout << "No new blocks, this means the disk is full" << std::endl;
	return -1;
}

//second parameter is used if 
int writeToBlock(int inodeToWrite, char type, int numToWrite, std::string strToWrite){
	FILE *disk = fopen(DISK_FILE, "r+");
	int sizeToWrite = 0;
	int sizeOfBlock = 0;
	//the inode index + type + 
	fseek(disk, inodeToWrite + sizeof(char), SEEK_SET);
	int blockIndex;
	//get the block index
	fread(&blockIndex, sizeof(int), 1, disk);
	//go to the block index + name
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	//if its a directory
	//Getthe number of bytes currently in the block
	fread(&sizeOfBlock, sizeof(int), 1, disk);
	if(numToWrite > 0){
		//type, inode, str length, string
		sizeToWrite = sizeOfBlock + sizeof(char)+ sizeof(int) +sizeof(int)+ strToWrite.length() + 1;
	}else{
		sizeToWrite = sizeOfBlock + strToWrite.length();
	}
	//go back to overwrite the number of bytes in the block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fwrite(&sizeToWrite, sizeof(int), 1, disk);
	//go to the old size of block
	fseek(disk, blockIndex + BLOCK_HEADER + sizeOfBlock, SEEK_SET);
	//write the type
	
	//write the type and inode index if block is directory 
	if(numToWrite > 0){
		//write the type
		fwrite(&type, sizeof(char), 1, disk);
		//write the inode index
		fwrite(&numToWrite, sizeof(int), 1, disk);
		//write the string length
		int strLen = strToWrite.length();
		fwrite(&strLen, sizeof(int), 1, disk);
		fwrite(&strToWrite[0], sizeof(char), strLen, disk);
	}
	//if its not a directory write the string
	else{
		fwrite(&strToWrite[0], sizeof(char), strToWrite.length(), disk);
	}
	//close
	fclose(disk);
	return 1;
}

int setCurrDir(int inode, std::string dir){
	currDir = dir;
	currDirInode = inode;
	return 1;
}

int openFile(std::string fname, int flag){
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	int fd = -1;
	char type;
	int inodeIndex;
	
	//go to the current directory in the inode table + type
	fseek(disk, currDirInode + sizeof(char), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1,disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for(int i = 0; i < blockSize; i+= 10+strLen){
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int),1,disk);
		char charFile[FILE_LENGTH];
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);	
		filename.resize(strLen);
		//std::cout << filename << fname;
		if(filename == fname && type == 'f'){
			//found the file check if its already open
			fd = checkIfFileOpen(inodeIndex);
			
			if(fd >= 0){
				std::cout << "File is already open with fd =" <<fd <<std::endl;
				fd = -2; 
				break;
			}
			//add it to the openFile array
			fd = addOpenFile(inodeIndex, flag);
			break;
		}
	}
	fclose(disk);
	//return the fd or -1 if didnt find file
	return fd;
}

int addOpenFile(int inode, int flag){
	for(int fd = 0; fd < 1024; fd++){
		if(openFiles[fd][0] < 0){
			openFiles[fd][0] = inode;
			openFiles[fd][1] = 0; //the file offset is 0 when we open
			openFiles[fd][2] = flag;
			return fd;
		}
	}
	//should always be a spot in openFiles[]
	std::cout << "NO MORE ROOM IN openFiles[]" <<std::endl;
	return -1;
}

int checkIfFileOpen(int inode){
	for(int fd = 0; fd < 1024; fd++){
		if(openFiles[fd][0] == inode){
			return fd;
		}
	}
	return -1;
}

int writeToFile(int fd, std::string str){
	int inode = openFiles[fd][0];
	int flag = openFiles[fd][2];

	//if the file is open with a flag of w or rw
	if (inode >= 0 && flag >= 2){
		writeToBlock(inode, 'f', 0, str);
		openFiles[fd][1] += str.length();
	}
	else{
		std::cout << "File is not open with a 'w' or 'rw' flag" << std::endl;
	}

	return 1;
}

int readFromFile(int fd, int size){

	int inode = openFiles[fd][0];
	int offset = openFiles[fd][1];
	int flag = openFiles[fd][2];
	int blockIndex;
	int blockSize;
	
	//if the file is open with a flag of w or rw
	if (inode >= 0 && (flag == 1 || flag == 3)){
		std::string output = readFromBlock(fd, inode, offset, size);
		std::cout << "OUTPUT: " << output << std::endl;
	}
	else{
		std::cout << "File is not open with a 'w' or 'rw' flag" << std::endl;
	}
	return 1;
}


std::string readFromBlock(int fd, int inode, int offset, int size, bool readAll){
	FILE *disk = fopen(DISK_FILE, "r+");
	int blockIndex;
	int sizeOfBlock;
	
 
	fseek(disk, inode + sizeof(char), SEEK_SET);
	//get the block index
	fread(&blockIndex, sizeof(int), 1, disk);
	//go to the block index + name
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);

	//Getthe number of bytes currently in the block
	fread(&sizeOfBlock, sizeof(int), 1, disk);
	if (readAll){
		//read all the bytes
		size = sizeOfBlock;
		offset = 0;
	}
	if (size + offset > sizeOfBlock){
		std::cout << "There are only " << sizeOfBlock - offset << " bytes left in the file. You can not read more than that" << std::endl;;
		size = sizeOfBlock - offset;
	}

	//go to the spot where we need to read from
	fseek(disk, blockIndex + BLOCK_HEADER + offset, SEEK_SET);
	char charFile[BLOCK_SIZE];
	fread(&charFile[0], sizeof(char), size, disk);
	std::string output(charFile);
	output.resize(size);
	openFiles[fd][1] += size;
	fclose(disk);
	return output;
}

int seekFile(int fd, int offset){

	int inode = openFiles[fd][0];
	int oldOffset = openFiles[fd][1];
	int blockIndex;
	int blockSize;

	//if the file is open with a flag of w or rw
	if (inode >= 0){
		openFiles[fd][1] = offset;
		std::cout << "FD " << fd << " offset is now " << offset << std::endl;
	}
	else{
		std::cout << "File is not open" << std::endl;
	}

	return 1;
}

int closeFile(int fd){
	int inode = openFiles[fd][0];
	int blockIndex;
	int blockSize;

	//if the file is open with a flag of w or rw
	if (inode >= 0){
		openFiles[fd][0] = -1;
		openFiles[fd][1] = 0; 
		openFiles[fd][2] = 0; 
		std::cout << "FILE " << fd <<" CLOSED" << std::endl;
	}
	else{
		std::cout << "That file is not open" << std::endl;
	}

	return 1;
}
