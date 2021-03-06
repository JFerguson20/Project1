#include "Disk.h"


std::string currDir = "";
int currDirInode = -1;
//start with all the open files to -1
int openFiles[1024][3];

int init(){
	setCurrDir(0, "/");
	for (int i = 0; i < 1024; i++){
		openFiles[i][0] = -1;
		openFiles[i][1] = 0; //the current offset
		openFiles[i][2] = 0; //file mode
	}
	return 1;
}

//INODE ENTRY
//CHAR - TYPE
//INT - BLOCK ADDRESS
//INT - BLOCK ADDRESS #2 FOR BIGGER FILES  * is there a #3?

/* type is either d for directory or f for file
*  name is to be passed to createBlock
*/
int createInode(char type, std::string name, int src, int flag){ // *creat all inodes 
	FILE *disk = fopen(DISK_FILE, "r+"); // *for read and write mode
	int linkCount = 1;
	int blockPtr1;
	int blockPtr2;
	char tempType; // for what?
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
			//write the linkCount
			fwrite(&linkCount, sizeof(int), 1, disk);
			//write of block1
			fwrite(&blockPtr1, sizeof(int), 1, disk);

			//write block2
			fwrite(&blockPtr2, sizeof(int), 1, disk);

			//if it was a file add to openFiles
			if (type == 'f'){
				fd = addOpenFile(i, flag);
			}
			//if it was a directory
			else if (type == 'd'){

			}

			//write this inode (i) to the current directorys block
			//if the current directory has all ready been set
			if (currDirInode >= 0){
				//write this inode (i) to the current directorys block
				if (type != 'l'){
					writeToBlock(currDirInode, type, i, name);
				}
				else{
					writeToBlock(currDirInode, type, src, name);
				}
			}
			else{
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
	int NumberOfBytes = 0; //when NumberOfBytes changes?
	int parentInode;

	//set the parent Inode
	if (currDirInode >= 0){
		parentInode = currDirInode;
	}
	else{
		parentInode = 0;
	}

	//number of links
	//int linkCount = 1; Moved to Inode
	//skip the inode table
	for (int i = NUMBER_INODES * INODE_SIZE; i < DISK_SIZE; i += BLOCK_SIZE){
		fseek(disk, i, SEEK_SET);
		fread(&tempBlock, sizeof(char), 1, disk);
		//if a block doesnt already exist here
		if (!tempBlock){
			//go back 1 before the read
			fseek(disk, i, SEEK_SET);

			fwrite(&name[0], sizeof(char), FILE_LENGTH, disk);//*write filename 
			fseek(disk, i + FILE_LENGTH, SEEK_SET);
			fwrite(&NumberOfBytes, sizeof(int), 1, disk);//write NumberOfBytes
			fwrite(&parentInode, sizeof(int), 1, disk);
			fclose(disk);


			return i;
		}
	}
	fclose(disk);
	std::cout << "No new blocks, this means the disk is full" << std::endl;
	return -1;
}

//numToWrite used if it is a directory block, offset is only used if it is a file
int writeToBlock(int inodeToWrite, char type, int numToWrite, std::string strToWrite, int offset){//inodeToWrite is the inode index?
	FILE *disk = fopen(DISK_FILE, "r+");
	int sizeToWrite = 0;
	int sizeOfBlock = 0;
	//the inode index + type +link count
	fseek(disk, inodeToWrite + sizeof(char) + sizeof(int), SEEK_SET); //char here is for type 
	int blockIndex;
	//get the block index
	fread(&blockIndex, sizeof(int), 1, disk);
	//go to the block index + name
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET); //the lenghth of name?  blockIndex * blockSize?

	//Getthe number of bytes currently in the block
	fread(&sizeOfBlock, sizeof(int), 1, disk);
	//if its a directory
	if (numToWrite > 0){
		//type, inode, str length, string
		sizeToWrite = sizeOfBlock + sizeof(char) + sizeof(int) + sizeof(int) + strToWrite.length() + 1;
	}
	else{
		sizeToWrite = sizeOfBlock + strToWrite.length();
	}
	//go back to overwrite the number of bytes in the block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fwrite(&sizeToWrite, sizeof(int), 1, disk);

	//close
	char blockArray1[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray1, sizeof(char), BLOCK_SIZE, disk);


	/*DIRECTORY FORMAT
	* BLOCK HEADER
	* -TYPE (char)
	* -INODE (int)
	* -FILENAME LENGTH (int)
	* -FILENAME (char*)
	*/
	if (numToWrite > 0){
		//go to the old size of block
		//might have to change this to go to the first open spot so when we delete files we dont eventually run out of room

		fseek(disk, blockIndex + BLOCK_HEADER + sizeOfBlock, SEEK_SET);
		//write the type
		fwrite(&type, sizeof(char), 1, disk);
		//write the inode index
		fwrite(&numToWrite, sizeof(int), 1, disk);
		//write the string length
		int strLen = strToWrite.length();
		fwrite(&strLen, sizeof(int), 1, disk);
		//write
		fwrite(&strToWrite[0], sizeof(char), strLen, disk);
	}
	//if its not a directory write the string, it is a file, then append the file to existing files
	else{
		fseek(disk, blockIndex + BLOCK_HEADER + offset, SEEK_SET);
		fwrite(&strToWrite[0], sizeof(char), strToWrite.length(), disk);
	}
	//close
	char blockArray[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray, sizeof(char), BLOCK_SIZE, disk);

	fclose(disk);
	return 1;
}

int setCurrDir(int inode, std::string dir){
	if (dir != ".."){
		if (dir == "/"){
			currDir = "/";
		}
		else{
			currDir += dir + "/";
		}
	}
	else{
		std::vector<std::string> v = split(currDir, "/");
		std::string newDir = "";

		for (int i = 0; i < v.size() - 2; ++i){
			newDir += v[i]+ "/";
		}
		currDir = newDir;
	}
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

	//go to the current directory in the inode table + type + link count
	fseek(disk, currDirInode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (filename == fname && (type == 'f' || type == 'l')){
			//found the file check if its already open
			fd = checkIfFileOpen(inodeIndex);

			if (fd >= 0){
				std::cout << "File is already open with fd =" << fd << std::endl;
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
	for (int fd = 0; fd < 1024; fd++){
		if (openFiles[fd][0] < 0){
			openFiles[fd][0] = inode;
			openFiles[fd][1] = 0; //the file offset is 0 when we open
			openFiles[fd][2] = flag;
			return fd;
		}
	}
	//should always be a spot in openFiles[]
	std::cout << "NO MORE ROOM IN openFiles[]" << std::endl;
	return -1;
}

int checkIfFileOpen(int inode){
	for (int fd = 0; fd < 1024; fd++){
		if (openFiles[fd][0] == inode){
			return fd;
		}
	}
	return -1;
}

int writeToFile(int fd, std::string str){
	int inode = openFiles[fd][0];
	int offset = openFiles[fd][1];
	int flag = openFiles[fd][2];

	//if the file is open with a flag of w or rw
	if (inode >= 0 && flag >= 2){
		writeToBlock(inode, 'f', 0, str, offset);
		openFiles[fd][1] += str.length();//openFiles[fd][1] = 0, when openning, now it becomes str.length?
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


	fseek(disk, inode + sizeof(char) + sizeof(int), SEEK_SET);
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
		std::cout << "There are only " << sizeOfBlock - offset << " bytes left in the file. You can not read more than that" << std::endl;
		size = sizeOfBlock - offset;
		openFiles[fd][1] += size; //current offset
	}

	//go to the spot where we need to read from
	fseek(disk, blockIndex + BLOCK_HEADER + offset, SEEK_SET);
	char charFile[BLOCK_SIZE];
	fread(&charFile[0], sizeof(char), size, disk);
	std::string output(charFile);
	output.resize(size);
	fclose(disk);
	return output;
}

int seekFile(int fd, int offset){ //havn't been used

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
		std::cout << "FILE " << fd << " CLOSED" << std::endl;
	}
	else{
		std::cout << "That file is not open" << std::endl;
	}

	return 1;
}

int createDir(int inode, std::string dirname){
	//Much of the code borrowed from openFile() to check if directory already exists
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, inode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;

	char blockArray[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray, sizeof(char), BLOCK_SIZE, disk);

	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == dirname && type == 'd'){
			//if the directory already exists
			fclose(disk);
			return -1;
		}
	}
	int tempInode = currDirInode;
	currDirInode = inode;
	createInode('d', dirname);
	currDirInode = tempInode;
	fclose(disk);
	//return the fd or -1 if didnt find file
	return 1;
}

int removeDir(std::string dirname){
	//see if directory exists, taken from create dir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, currDirInode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);

	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == dirname && type == 'd'){
			if (isEmpty(inodeIndex)){
				//if the directory is empty then erase

				eraseFileFromDir(blockIndex, blockIndex + BLOCK_HEADER + i);
				eraseInode(inodeIndex);
			}
			//if the directory already exists
			fclose(disk);
			return 1;
		}
	}
	fclose(disk);
	//return the fd or -1 if didnt find file
	return -1;
}

//returns inodes of new directory
int changeDir(int inode, std::string dirname){
	//see if directory exists, taken from create dir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;
	int parentInode;
	//go to the current directory in the inode table + type + link count
	fseek(disk, inode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);

	if (dirname == ".."){
		fread(&parentInode, sizeof(int), 1, disk);
		fclose(disk);
		return parentInode;
	}

	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == dirname && type == 'd'){
			//if the directory exists
			fclose(disk);
			return inodeIndex;
		}
	}

	int tempInode = currDirInode;
	currDirInode = inode;
	createInode('d', dirname);
	currDirInode = tempInode;

	fclose(disk);
	//return the fd or -1 if didnt find file
	return 1;
}

int ls(){
	//much code taken from createdir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, currDirInode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);


	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;

	char blockArray[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray, sizeof(char), BLOCK_SIZE, disk);

	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		std::cout << filename << "     ";
	}
	std::cout << std::endl;
	fclose(disk);
	//return the fd or -1 if didnt find file

	return 1;
}

int linkFiles(int srcInode, int destInode, std::string src, std::string dest){

	//see if directory exists, taken from create dir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;
	int parentInode;
	//go to the current directory in the inode table + type + link count
	fseek(disk, srcInode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);

	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == src && type == 'f'){
			//if the srouce exists

			int tempInode = currDirInode;
			currDirInode = destInode;
			createInode('l', dest, inodeIndex, 0);
			currDirInode = tempInode;
			incrementLink(inodeIndex);
			fclose(disk);
			return inodeIndex;
		}
	}
	std::cout << "That source doesnt exists" << std::endl;
	fclose(disk);
	//return the fd or -1 if didnt find file

	return 1;
}

int unlinkFiles(int inode, std::string dest){

	//see if directory exists, taken from create dir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;
	int parentInode;
	//go to the current directory in the inode table + type + link count
	fseek(disk, inode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);

	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;

	//go through the block
	for (int i = 0; i < blockSize; i += 10 + strLen){
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == dest && type == 'l'){
			//if the srouce exists
			decrementLink(inodeIndex);
			//erase this link
			eraseFileFromDir(blockIndex, blockIndex + BLOCK_HEADER + i);
			eraseInode(inodeIndex);
			fclose(disk);
			return inodeIndex;
		}
	}
	std::cout << "That link doesnt exists" << std::endl;
	fclose(disk);
	return 1;
}

int stats(std::string fname){
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	int linkCount;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type 
	fseek(disk, currDirInode + sizeof(char), SEEK_SET);
	fread(&linkCount, sizeof(int), 1, disk);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (filename == fname){


			std::cout << "NAME: " << fname << std::endl;
			if (type == 'd'){
				std::cout << "TYPE: Directory" << std::endl;
			}
			else if (type == 'f'){
				std::cout << "TYPE: File" << std::endl;
			}
			else{
				std::cout << "TYPE: Link" << std::endl;
			}
			std::cout << "INODE: " << inodeIndex << std::endl;
			std::cout << "LINK COUNT: " << linkCount << std::endl;

			// go to the file inode
			fseek(disk, inodeIndex + sizeof(char) + sizeof(int), SEEK_SET);
			fread(&blockIndex, sizeof(int), 1, disk);
			std::cout << "BLOCK INDEX: " << blockIndex << std::endl;
			fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
			fread(&blockSize, sizeof(int), 1, disk);
			std::cout << "SIZE: " << blockSize << " bytes" << std::endl;

			std::cout << "NUM BLOCKS: 1" << std::endl;
			fclose(disk);
			return 1;
		}
	}
	std::cout << "File does not exist" << std::endl;

	fclose(disk);


	return 1;
}

int catFile(std::string fname){

	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, currDirInode + sizeof(char) + sizeof(int), SEEK_SET);

	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (filename == fname && (type == 'f' || type == 'l')){
			fclose(disk);
			std::string output = readFromBlock(0, inodeIndex, 0, 0, true);
			std::cout << output << std::endl;

			return 1;
		}
	}
	std::cout << "File does not exist" << std::endl;

	fclose(disk);

	return 1;
}

int printTree(int dirInode, int depth){

	//much code taken from createdir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, dirInode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);


	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;

	char blockArray[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray, sizeof(char), BLOCK_SIZE, disk);

	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (type == 'd'){
			for (int d = 0; d < depth; d++){
				std::cout << "-";
			}
			std::cout << filename << std::endl;
			printTree(inodeIndex, depth + 1);
		}
		else{
			for (int d = 0; d < depth; d++){
				std::cout << "-";
			}
			// go to the file inode
			int tempBlockIndex;
			fseek(disk, inodeIndex + sizeof(char) + sizeof(int), SEEK_SET);
			fread(&tempBlockIndex, sizeof(int), 1, disk);
			fseek(disk, tempBlockIndex + FILE_LENGTH, SEEK_SET);
			fread(&blockSize, sizeof(int), 1, disk);
			std::cout << filename << " " << blockSize << std::endl;
		}
	}

	fclose(disk);
	//return the fd or -1 if didnt find file

	return 1;
}

int importFile(std::string src, std::string dest){

	int flag = 2; //for write
	int fd = createInode('f', dest, 0, flag);
	int inode = openFiles[fd][0];
	//read file line by line are write the file
	std::ifstream file(src.c_str());
	std::string str;
	std::string file_contents;
	int offset = 0;
	//write line by line
	while (std::getline(file, str))
	{
		str.push_back('\n');
		writeToBlock(inode, 'f', 0, str, offset);
		offset += str.length();
	}
	closeFile(fd);
	file.close();
	return 1;
}


//TAKEN FROM CAT BUT INSTEAD OF PRINTING TO CONSOLE PRINT TO FILE
int exportFile(std::string src, std::string dest){

	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, currDirInode + sizeof(char) + sizeof(int), SEEK_SET);

	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (filename == src && (type == 'f' || type == 'l')){
			fclose(disk);
			std::string output = readFromBlock(0, inodeIndex, 0, 0, true);
			std::ofstream out(dest.c_str());
			out << output;
			out.close();
			std::cout << "File exported" << std::endl;

			return 1;
		}
	}
	std::cout << "Src does not exist" << std::endl;

	fclose(disk);

	return 1;
}

int copy(int srcInode, int destInode, std::string src, std::string dest){

	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;

	//go to the current directory in the inode table + type + link count
	fseek(disk, srcInode + sizeof(char) + sizeof(int), SEEK_SET);

	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);
	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);

		if (filename == src && type == 'f'){
			
			//Have the contents of the file
			std::string output = readFromBlock(0, inodeIndex, 0, 0, true);
			
			int flag = 2; //for write

			int tempInode = currDirInode;
			currDirInode = destInode;
			int fd = createInode('f', dest, 0, flag);
			currDirInode = tempInode;
			int inode = openFiles[fd][0];
			//read file line by line are write the file
			int offset = 0;
			//write line by line
			writeToBlock(inode, 'f', 0, output, 0);
				
			closeFile(fd);
			fclose(disk);
			return 1;
		}
	}
	std::cout << "File does not exist" << std::endl;

	fclose(disk);


	return 1;

}

int numOfBlocks(int inodeIndex){
	int ret = 1;
	return ret;
}

void incrementLink(int inode){
	FILE *disk = fopen(DISK_FILE, "r+");
	//go to the 
	int linkCount;
	fseek(disk, inode + sizeof(char), SEEK_SET);
	fread(&linkCount, sizeof(int), 1, disk);
	linkCount++;
	fseek(disk, inode + sizeof(char), SEEK_SET);
	fwrite(&linkCount, sizeof(int), 1, disk);
	fclose(disk);
}

void decrementLink(int inode){
	FILE *disk = fopen(DISK_FILE, "r+");
	//go to the 
	int linkCount;
	int blockIndex;
	int deleteIndex;
	fseek(disk, inode + sizeof(char), SEEK_SET);
	fread(&linkCount, sizeof(int), 1, disk);
	linkCount--;

	if (linkCount == 0){
		//eraseFileFromDir(blockIndex, deleteIndex);
		eraseInode(inode);
	}
	else{
		fseek(disk, inode + sizeof(char), SEEK_SET);
		fwrite(&linkCount, sizeof(int), 1, disk);
	}

	fclose(disk);
}

int eraseFileFromDir(int blockIndex, int deleteIndex){
	//much code taken from createdir
	FILE *disk = fopen(DISK_FILE, "r+");

	int delLength;

	char blockArray[BLOCK_SIZE];
	fseek(disk, blockIndex, SEEK_SET);
	fread(blockArray, sizeof(char), BLOCK_SIZE, disk);

	//get length of entry we are deleting
	fseek(disk, deleteIndex + 5, SEEK_SET);
	fread(&delLength, sizeof(int), 1, disk);
	delLength += 10;
	for (int i = deleteIndex; i < blockIndex + BLOCK_SIZE - delLength; i++){
		//scoot everything in array up
		int iTemp = i - blockIndex;
		blockArray[iTemp] = blockArray[iTemp + delLength];



	}
	fseek(disk, blockIndex, SEEK_SET);
	//write the update block
	fwrite(blockArray, sizeof(char), BLOCK_SIZE, disk);

	//UPDATE SIZE OF BLOCK
	int sizeOfBlock;
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	//Getthe number of bytes currently in the block
	fread(&sizeOfBlock, sizeof(int), 1, disk);
	//update the size
	sizeOfBlock = sizeOfBlock - delLength;
	//go back to overwrite the number of bytes in the block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fwrite(&sizeOfBlock, sizeof(int), 1, disk);
	fclose(disk);
	return 1;
}

void eraseInode(int inode){
	FILE *disk = fopen(DISK_FILE, "r+");
	// go to the inode spot
	fseek(disk, inode, SEEK_SET);
	char zeros[] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
	fwrite(zeros, sizeof(char), 14, disk);
	fclose(disk);
}

//checks if a directory is empty
bool isEmpty(int inode){

	return true;
}

std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty) {
	std::vector<std::string> result;
	if (delim.empty()) {
		result.push_back(s);
		return result;
	}
	std::string::const_iterator substart = s.begin(), subend;
	while (true) {
		subend = search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			result.push_back(temp);
		}
		if (subend == s.end()) {
			break;
		}
		substart = subend + delim.size();
	}
	return result;
}



//takes a input dir1/dir2 and gives 
int findDirInode(std::string input){
	// we define a string
	std::vector<std::string> v = split(input, "/");
	int dirInode = currDirInode;
	for (int i = 0; i < v.size() -1 ; ++i ){
		dirInode = findDir(dirInode, v[i]);
		if (dirInode < 0){
			std::cout << "could not find dir " << v[i] << std::endl;
			break;
		}

	}

	return dirInode;
}

int findDir(int inode, std::string dirname){

	//see if directory exists, taken from create dir
	FILE *disk = fopen(DISK_FILE, "r+");
	//search through the current directory for the filename
	int blockIndex;
	int blockSize;
	char type;
	int inodeIndex;
	int parentInode;
	//go to the current directory in the inode table + type + link count
	fseek(disk, inode + sizeof(char) + sizeof(int), SEEK_SET);
	fread(&blockIndex, sizeof(int), 1, disk);
	//go the the curr directory block
	fseek(disk, blockIndex + FILE_LENGTH, SEEK_SET);
	fread(&blockSize, sizeof(int), 1, disk);

	if (dirname == ".."){
		fread(&parentInode, sizeof(int), 1, disk);
		fclose(disk);
		return parentInode;
	}

	//check the block for the filename (type,inode index, filename)
	fseek(disk, blockIndex + BLOCK_HEADER, SEEK_SET);
	int strLen = 0;
	for (int i = 0; i < blockSize; i += 10 + strLen){ // 10 = inodesize?
		fseek(disk, blockIndex + BLOCK_HEADER + i, SEEK_SET);
		fread(&type, sizeof(char), 1, disk);
		fread(&inodeIndex, sizeof(int), 1, disk);
		fread(&strLen, sizeof(int), 1, disk);
		char charFile[FILE_LENGTH];//to store the file name?
		fread(&charFile[0], sizeof(char), strLen, disk);
		std::string filename(charFile);
		filename.resize(strLen);
		//std::cout << filename << fname;
		if (filename == dirname && type == 'd'){
			//if the directory exists
			fclose(disk);
			return inodeIndex;
		}
	}
	fclose(disk);
	//return the fd or -1 if didnt find file
	return -1;

}
