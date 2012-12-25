//
//  main.cpp
//  cppnzb
//
//  Created by Johan Attali on 12/15/12.
//  Copyright (c) 2012 Johan Attali. All rights reserved.
//

#include <iostream>

using std::cout;
using std::cin;
using std::endl;
#include "nntp.h"

int main(int argc, const char * argv[])
{
  nntp::nntp nntp;
  bool connected  = nntp.secureConnect("ssl-eu.astraweb.com");
  bool logged     = nntp.login("apouche", "C0mm4nche");
  
  if (connected && logged)
  {
    cout << "Connected" << endl;
    
    std::string output;
    
    while (1)
    {
      cout << "> ";
      
      std::string command;
      
      std::getline(std::cin, command);  // read a line from std::cin into line
          
      nntp.process_block_string(command + "\n", output);
      std::cout << output << std::endl;
      
    }
    
    
  }
  
  return 0;
}

