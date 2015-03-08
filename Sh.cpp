#include "Sh.h"

/*
 * Parses testinput file and  calls the corresponding command
 * passes all parameters to the commands for processing
 */ 
int parseCommandForSh(std::string command){
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
  else if(str == "mkdir"){
    mkdir(tokens);
  }
  else if(str == "rmdir"){
    rmdir(tokens);
  }
  else if(str == "cd"){
    cd(tokens);
  }
  else if(str == "ls"){
    ls(tokens);
  }
  else if (str == "link"){
    link(tokens);
  }
  else if (str == "unlink"){
    unlink(tokens);
  }
  else if (str == "cat"){
    cat(tokens);
  }
  else if (str == "stat"){
    stat(tokens);
  }
  else if (str == "tree"){
    tree(tokens);
  }
  else if (str == "import"){
    import(tokens);
  }
  else if (str == "export"){
    expo(tokens);
  }
   else if (str == "cp"){
	 cp(tokens); 
  } 
  else if (str == "exit"){
    return -1; 
  }

  else 
  {
    std::cout << "Command not implemented yet or command is invalid" << std::endl;
    return -1;
  }
}


/*
 *Add sh cammand
 */
int sh(std::vector<std::string> params){
  if(params.size() == 3){

    std::ifstream infile (params[1].c_str());
    std::string line;
    //begin redirection
    //std::fstream ofs("b.txt"); 
    std::fstream ofs(params[2].c_str(), std::ofstream::out); 
    std::streambuf *osb = std::cout.rdbuf(ofs.rdbuf()); 
    while (std::getline(infile, line))
    {
      //parse command line by line
      parseCommandForSh(line);
    }
    //end redirection
    std::cout.rdbuf(osb);
    return 1;
  }else 
    return -1;
}


/*
 * int shRemote(std::vector<std::string> params){
  if(params.size() == 3){

    std::ifstream infile (params[1].c_str());
    std::string line;
    while (std::getline(infile, line))
    {
      parseCommandForSh(line);
    }
    return 1;
  }else 
    return -1;
}
*/

