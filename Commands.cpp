#include "Commands.h"

/*there is no argument for mkfs shell commands*/
int mkfs(std::vector<std::string> params){
	if(params.size() == 1){
		//creates new file to act as the virtual disk
		FILE *disk = fopen(DISK_FILE, "w+");
		//seek to the disk size and write a 0 to make the file 100MBs
		fseek(disk, DISK_SIZE, SEEK_SET);
		char end = '0'; //write 0 to disk
		fwrite(&end, sizeof(char), 1, disk);
		//close the disk
		fclose(disk);
		std::cout << "Virtual Disk created" << std::endl;
		//create inode for the home directory
		std::string name = "/";
		createInode('d', name);
		
		return 1;
	}else{
		std::cout << "MKFS does not have any parameters" << std::endl;
		return -1;
	}
}


int open(std::vector<std::string> params){
	if(params.size() == 3){ //3 or 2?
		std::string filename = params[1];
		int fd;
		int flag;
		//figure out the flag.. 1:r, 2:w, 3:rw
		if(params[2] == "r"){
			flag = 1;
		}else if(params[2] == "w"){
			flag = 2;
		}else if(params[2] == "rw"){
			flag = 3;
		}else{
			std::cout << "FLAG MUST BE r w OR rw" <<std::endl;
			return 1;
		}
		
		//check if file name is less than 16 characters
		if(filename.length() < FILE_LENGTH){
			//see if the file already exists
			fd = openFile(filename, flag); 
			//create the file if it doesnt exist
			if(fd == -1){
				if (flag == 2 || flag == 3){ // w or rw
					fd = createInode('f', filename, 0, flag);
				}
				else{
					std::cout << "File does not exist, open with 'w' to create file" << std::endl;
					return -1;
				}
			}else if(fd == -2){ // file already is open
				return -1;
			}
			
			//print the fd
			std::cout << "SUCCESS, fd=" << fd << std::endl;
		}else{
			std::cout << "File name must be 16 or less characters" <<std::endl;
		}
		
		
		 
	}else{
		std::cout << "OPEN has two parameters <filename> <flag>" << std::endl;
	}
	return 1;
}

int read(std::vector<std::string> params){
	if(params.size() == 3){
	
		std::stringstream ss(params[1]);
		int fd;
		//error checking
		if ((ss >> fd).fail()){
			std::cout << "fd should be a integer" << std::endl;
			return 1;
		}
		if (fd >= 1024){
			std::cout << "that fd does not exist" << std::endl;
			return 1;
		}
		
		std::stringstream ss2(params[2]);
		int size;
		if ((ss2 >> size).fail()){
			std::cout << "size should be a integer" << std::endl;
			return 1;
		}
		if (size <= 0){
			std::cout << "size must be positive" << std::endl;
			return 1;
		}

		readFromFile(fd, size);		
	}else{
		std::cout << "READ has two parameters <fd> <size>" << std::endl;
	}
	return 1;
}

int write(std::vector<std::string> params, std::string str){
	if(params.size() >= 3){
		std::stringstream ss(params[1]);
		int fd;
		//error checking
		if(( ss >> fd).fail()){
			std::cout << "fd should be a integer" <<std::endl;
			return 1;
		}
		if(fd >= 1024){
			std::cout << "that fd does not exist" <<std::endl;
			return 1;
		}

		std::string firstWord = params[2];
		std::string lastWord = params[params.size() - 1];
		//make sure the string ends and starts with a "
		if (firstWord.at(0) != '"' || lastWord.at(lastWord.length() - 1) != '"'){
			std::cout << "The string you want to write must be in quotations" << std::endl;
		}
		else{
			//find first and last of " " 
			int first = str.find_first_of("\"") + 1;
			int last = str.find_last_of("\"");
			//get substring between the " "
			std::string substr = str.substr(first, last - first);
			int ret = writeToFile(fd, substr);
		}

		
		
	}else{
		std::cout << "WRITE has two parameters <fd> <\"string\">" << std::endl;
	}
	return 1;
}

int seek(std::vector<std::string> params){
	if(params.size() == 3){
		std::cout << "SEEK" << std::endl;
		//fd = params[1]
		//offset = params[2]
		std::stringstream ss(params[1]);
		int fd;
		//error checking
		if ((ss >> fd).fail()){
			std::cout << "fd should be a integer" << std::endl;
			return 1;
		}
		if (fd >= 1024){
			std::cout << "that fd does not exist" << std::endl;
			return 1;
		}

		std::stringstream ss2(params[2]);
		int offset;
		if ((ss2 >> offset).fail()){
			std::cout << "size should be a integer" << std::endl;
			return 1;
		}
		if (offset < 0){
			std::cout << "offset must be positive" << std::endl;
			return 1;
		}

		seekFile(fd, offset);
	}else{
		std::cout << "SEEK has two parameters <fd> <offset>" << std::endl;
	}
	return 1;
}

int close(std::vector<std::string> params){
	if(params.size() == 2){
		std::stringstream ss(params[1]);
		int fd;
		//error checking
		if ((ss >> fd).fail()){
			std::cout << "fd should be a integer" << std::endl;
			return 1;
		}
		if (fd >= 1024){
			std::cout << "that fd does not exist" << std::endl;
			return 1;
		}
		
		closeFile(fd);
	}else{
		std::cout << "Close has one parameter <fd>" << std::endl;
	}
	return 1;
}

int mkdir(std::vector<std::string> params){
	if(params.size() == 2){
		
		int inode = findDirInode(params[1]);
		//check if file name is less than 16 characters
		std::vector<std::string> v = split(params[1], "/");
		std::string dirname = v[v.size() - 1];
		if(dirname.length() < FILE_LENGTH){
			if(createDir(inode, dirname) < 0){
				std::cout << "File name already exists" <<std::endl;
			}
		}else{
			std::cout << "File name is to large, max of 16 characters" << std::endl;
		}
		
		return 1;
	}else{
		std::cout << "mkdir has one parameter <dirname>" << std::endl;
		return -1;
	}
}

int rmdir(std::vector<std::string> params){
	if(params.size() == 2){


		int inode = findDirInode(params[1]);
		//check if file name is less than 16 characters
		std::vector<std::string> v = split(params[1], "/");
		std::string dirname = v[v.size() - 1];
		if(dirname.length() < FILE_LENGTH){
			if (changeDir(inode, dirname) >= 0){
				removeDir(dirname);
			}
			else{
				std::cout << "Directory does not exist" << std::endl;
			}
		}
		else{
			std::cout << "File name is to large, max of 16 characters" << std::endl;
		}
		return 1;
	}else{
		std::cout << "rmdir has one parameter <dirname>" << std::endl;
		return -1;
	}
}

int cd(std::vector<std::string> params){
	if(params.size() == 2){

		int inode = findDirInode(params[1]);
		//check if file name is less than 16 characters
		std::vector<std::string> v = split(params[1], "/");
		std::string dirname = v[v.size() - 1];
		if(dirname.length() < FILE_LENGTH){
			int newDir = changeDir(inode, dirname);
			if (newDir >= 0){
				setCurrDir(newDir, dirname);
			}else{
				std::cout << "File name already exists" << std::endl;
			}
		}
		else{
			std::cout << "File name is to large, max of 16 characters" << std::endl;
		}

		return 1;
	}else{
		std::cout << "cd has one parameter <dirname>" << std::endl;
		return -1;
	}
}

int ls(std::vector<std::string> params){
	if(params.size() == 1){

		ls();
		return 1;
	}else{
		std::cout << "ls has no parameters" << std::endl;
		return -1;
	}
}

int link(std::vector<std::string> params){
	if (params.size() == 3){

		//
		int srcInode = findDirInode(params[1]);
		int destInode = findDirInode(params[2]);
		//check if file name is less than 16 characters
		std::vector<std::string> vs = split(params[1], "/");
		std::string src = vs[vs.size() - 1];

		std::vector<std::string> vd = split(params[2], "/");
		std::string dest = vd[vd.size() - 1];

		if(src.length() < FILE_LENGTH || dest.length() < FILE_LENGTH){
			//check if file name is less than 16 characters
			
			linkFiles(srcInode, destInode, src, dest);
		}else{
			std::cout << "File name is to large, max of 16 characters" << std::endl;
		}
		return 1;
	}
	else{
		std::cout << "link has two parameters <src> <dest>" << std::endl;
		return -1;
	}
}

int unlink(std::vector<std::string> params){
	if (params.size() == 2){

		//check if file name is less than 16 characters
		int destInode = findDirInode(params[1]);
		std::vector<std::string> vd = split(params[1], "/");
		std::string dest = vd[vd.size() - 1];
		unlinkFiles(destInode, dest);

		return 1;
	}
	else{
		std::cout << "unlink has 1 parameter <dest>" << std::endl;
		return -1;
	}
}

int stat(std::vector<std::string> params){
	if (params.size() == 2){
		std::string filename = params[1];
		//check if file name is less than 16 characters

		stats(filename);

		return 1;
	}
	else{
		std::cout << "stat has 1 parameter <filename>" << std::endl;
		return -1;
	}
}

int cat(std::vector<std::string> params){
	if (params.size() == 2){
		std::string filename = params[1];
		//check if file name is less than 16 characters

		catFile(filename);



		return 1;
	}
	else{
		std::cout << "cat has 1 parameter <filename>" << std::endl;
		return -1;
	}
}

int tree(std::vector<std::string> params){
	if (params.size() == 1){
		printTree(currDirInode, 0);
	}
	else{
		std::cout << "tree has no parameters" << std::endl;
		return -1;
	}
}

int import(std::vector<std::string> params){
	if (params.size() == 3){
		std::string src = params[1];
		std::string dest = params[2];
		importFile(src, dest);
	}
	else{
		std::cout << "import has two parameters <src> <dest>" << std::endl;
		return -1;
	}
}


int expo(std::vector<std::string> params){
	if (params.size() == 3){
		std::string src = params[1];
		std::string dest = params[2];
		exportFile(src, dest);
	}
	else{
		std::cout << "export has two parameters <src> <dest>" << std::endl;
		return -1;
	}
}

int cp(std::vector<std::string> params){
	if (params.size() == 3){

		int srcInode = findDirInode(params[1]);
		int destInode = findDirInode(params[2]);
		//check if file name is less than 16 characters
		std::vector<std::string> vs = split(params[1], "/");
		std::string src = vs[vs.size() - 1];

		std::vector<std::string> vd = split(params[2], "/");
		std::string dest = vd[vd.size() - 1];
;
		copy(srcInode, destInode, src, dest);
	}
	return 1;
}
