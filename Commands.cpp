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
					fd = createInode('f', filename, flag);
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

//ADD REST OF COMMANDS HERE


