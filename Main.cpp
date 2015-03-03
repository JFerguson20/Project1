
#include "Main.h"

std::string prompt = "Feng-Ferguson:~";


int main(void) {
	//loop for the prompt 
	init();
	while (1) {
		std::string command;
		std::cout << prompt + currDir + "$ ";
	
		if (!std::getline(std::cin, command)) {
			//error
			return -1;
		}
		//trim away leading or trailing spaces
		trim(command);
		//if they typed something trim then parse it
		if (!command.empty()) {
			// will return -1 to exit
			// else keep looping
			if (parseCommand(command) == -1){
				break;
			}
		}
	}
	return 1;
}

/*
 * Parses the user input and calls the corresponding command
 * passes all parameters to the commands for processing
 */ 
int parseCommand(std::string command){
	//This spilts the string on spaces into the vector tokens
	std::string buf;
	std::stringstream ss(command);
	//tokens holds the command as well as all the parameters
	std::vector<std::string> tokens; 
	while (ss >> buf){
		tokens.push_back(buf);
	}
	//get the first word from tokens
	std::string str = tokens[0];
	//see if the string matches our commands
	if(str == "mkfs"){
		mkfs(tokens);
	}
	else if(str == "open"){
		open(tokens);
	}
	else if(str == "read"){
		read(tokens);
	}
	else if(str == "write"){
		write(tokens, command);
	}
	else if(str == "seek"){
		seek(tokens);
	}
	else if(str == "close"){
		close(tokens);
	}
	//TODO
	//Add the rest of commands
	else if(str == "exit"){
		return -1; 
	}else{
		std::cout << "Command not implemented yet or command is invalid" << std::endl;
	}
	
}

/*
 * Trims any leading or trailing spaces
 */
void trim(std::string& str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
	if(pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != std::string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}
