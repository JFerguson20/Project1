/*
 * server & client
 * */
#include "Remote.h"


/*
 *if sh command or not
 */

bool isSh(std::string command){
  //This spilts the string on spaces into the vector tokens
  std::string buf;
  std::stringstream ss(command);
  //tokens holds the command as well as all the parameters
  std::vector<std::string> tokens; 
  while (ss >> buf){
    tokens.push_back(buf);
  }
  std::string str = tokens[0];
  //see if the string matches our commands
  if(str == "sh"){
    return true;
  }
  else
    return false;
}




/*
 * Parses the user input and calls the corresponding command
 * passes all parameters to the commands for processing
 */ 
int parseRemoteCommand(std::string command){
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

  //Add end here
  //TODO
  //Add the rest of commands
  else if (str == "sh"){
    sh(tokens);
  }
  else if(str == "exit"){
    return -1; 
  }else{
    std::cout << "Command not implemented yet or command is invalid" << std::endl;
  }

}



/*
 *Remote cammand
 */
int remote(std::vector<std::string> params){

  //variable for both server and client
  int sock_ser_fd, sock_cli_fd, newsock_ser_fd, portno_ser, portno_cli;
  socklen_t clilen;
  char buffer_ser[256], buffer_cli[256];
  struct sockaddr_in serv_addr_local, serv_addr_remote, cli_addr;
  struct hostent *server;
  int n_ser, n_cli;

  //server
  if (params[0] == "server"){
    if (params.size() == 2){
      //new a scoket
      sock_ser_fd = socket(AF_INET, SOCK_STREAM, 0);
      //new a scoket failed
      if (sock_ser_fd < 0){ 
        perror("error opening socket");
        exit(1);
      }
      //flush buffer_ser 
      bzero((char *) &serv_addr_local, sizeof(serv_addr_local));
      //get port number
      portno_ser = atoi( params[1].c_str());

      //set server address structure
      serv_addr_local.sin_family = AF_INET;
      serv_addr_local.sin_addr.s_addr = INADDR_ANY;
      serv_addr_local.sin_port = htons(portno_ser);
      //bind socket to server_address

      int opt = 1;
      setsockopt(sock_ser_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
      if (bind(sock_ser_fd, (struct sockaddr *) &serv_addr_local,
            sizeof(serv_addr_local)) < 0) {
        perror("error socket binding");
        exit(1);
      }
      int parse_result = 1;
      while (parse_result != -1){
        //listen on the socket for connection
        listen(sock_ser_fd,5);
        clilen = sizeof(cli_addr);
        //wake up the process when a connection established
        newsock_ser_fd = accept(sock_ser_fd, 
            (struct sockaddr *) &cli_addr, 
            &clilen);
        if (newsock_ser_fd < 0) {
          perror("error on accept");
          exit(1);
        }
        //flush buffer_ser
        bzero(buffer_ser,256);
        //read and print a message sent from client
        n_ser = read(newsock_ser_fd,buffer_ser,255);
        if (n_ser < 0) {
          perror("error reading from socket");
          exit(1);
        }
        std::cout << "Command received from client: ";
        std::cout << buffer_ser << std::endl;
        //send a message to client 

        //begin redirection
        std::string str_ser(buffer_ser);
        if (isSh(str_ser)){
          parse_result = parseRemoteCommand(str_ser);
          std::string str_wr = "Outputs have been stored at the server machine";
          const char *buf = str_wr.c_str();
          write(newsock_ser_fd,buf,strlen(buf));
        } else {
          std::fstream ofs("remote.dat", std::ofstream::out); 
          std::streambuf *osb = std::cout.rdbuf(ofs.rdbuf());
          //Add command parsing here
          parse_result = parseRemoteCommand(str_ser);
          //end redirection
          std::cout.rdbuf(osb);

          char buffer[256]; 
          std::fstream out; 
          out.open("remote.dat",std::ios::in); 
          while(!out.eof()) 
          { 
            out.getline(buffer,256,'\n');
            //        cout<<buffer<<endl; 
            n_ser = write(newsock_ser_fd,buffer,strlen(buffer));
            if (n_ser < 0) {
              perror("error writing to socket");
              exit(1);
            }
          }
        }
      }

      //close sockets
      //else{
      close(newsock_ser_fd);
      close(sock_ser_fd);
      return 0; 
      // }
    }
  }
  //client
  else if (params[0] == "client"){
    if (params.size() == 3 ){
      //port number
      portno_cli = atoi(params[1].c_str());
      //new a socket
      sock_cli_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (sock_cli_fd < 0) {
        perror("error on opening socket");
        exit(1);
      }
      //server name
      const char *server_name = params[2].c_str();
      server = gethostbyname(server_name);
      //server = gethostbyname(argv[1]);
      if (server == NULL) {
        fprintf(stderr,"error, no such host\n");
        exit(0);
      }
      //flush and set server_addr
      bzero((char *) &serv_addr_remote, sizeof(serv_addr_remote));
      serv_addr_remote.sin_family = AF_INET;
      bcopy((char *)server->h_addr, 
          (char *)&serv_addr_remote.sin_addr.s_addr,
          server->h_length);
      serv_addr_remote.sin_port = htons(portno_cli);

      //setup a connection from client to server
      if (connect(sock_cli_fd,(struct sockaddr *) &serv_addr_remote,sizeof(serv_addr_remote)) < 0) {
        perror("error connecting");
        exit(1);
      }
      std::string str_mg;  
      std::cout << "input command: ";
      //    str_mg = "";
      getline(std::cin, str_mg);
      //    while (str_mg != "exit"){

      bzero(buffer_cli,256);
      strcpy(buffer_cli, str_mg.c_str());

      //send a message to server
      n_cli = write(sock_cli_fd,buffer_cli,strlen(buffer_cli));
      if (n_cli < 0) {
        perror("error writing to socket");
        exit(1);
      }
      bzero(buffer_cli,256);
      //receive a message sent back by server
      n_cli = read(sock_cli_fd,buffer_cli,255);
      if (n_cli < 0) {
        perror("error reading from socket");
        exit(1);
      }

      std::cout << buffer_cli << std::endl;

      close(sock_cli_fd);
      return 1;
    }
      else {
        std::cout << "the number of client command's parameters is incorrect."
          << std::endl;
        return 0;
      }
    }
  }
