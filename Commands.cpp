#include "Commands.h"

int mkfs(std::vector<std::string> params){
	if(params.size() == 1){
		std::cout << "MKFS" << std::endl;
	}else{
		std::cout << "MKFS does not have any parameters" << std::endl;
	}
}

int open(std::vector<std::string> params){
	if(params.size() == 3){
		std::cout << "OPEN" << std::endl;
		//filename = params[1]
		//flag = params[2]
	}else{
		std::cout << "OPEN has two parameters <filename> <flag>" << std::endl;
	}
}

int read(std::vector<std::string> params){
	if(params.size() == 3){
		std::cout << "READ" << std::endl;
		//fd = params[1]
		//size = params[2]
	}else{
		std::cout << "READ has two parameters <fd> <size>" << std::endl;
	}
}

int write(std::vector<std::string> params){
	if(params.size() >= 3){
		std::cout << "WRITE" << std::endl;
		//fd = params[1]
		//string = params[2] // this is only first word of string, we have to loop through rest of params
	}else{
		std::cout << "WRITE has two parameters <fd> <\"string\">" << std::endl;
	}
}

int seek(std::vector<std::string> params){
	if(params.size() == 3){
		std::cout << "SEEK" << std::endl;
		//fd = params[1]
		//offset = params[2]
	}else{
		std::cout << "SEEK has two parameters <fd> <offset>" << std::endl;
	}
}

//ADD REST OF COMMANDS HERE
