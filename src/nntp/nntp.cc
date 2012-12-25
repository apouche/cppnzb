/**
 * This file is part of libnntp.
 *
 * libnntp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libnntp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libnntp. If not, see <http://www.gnu.org/licenses/>.
 */


#include "nntp.h"
#include "group.h"

namespace nntp
{
  // initialize socket and buffer
  void nntp::initialize()
  {
    // no data is in the buffer yet
    position        =   buffer;
    
    // make sure that our buffer is empty
    buffer[0]       =   '\0';
  }
  
  // default constructor
  nntp::nntp()
  {
    // initialize ourselves
    initialize();
  }
  
  // destructor that closes the connection and does some housekeeping
  nntp::~nntp()
  {
    // see ya!
    disconnect();
  }
  
  // make a connection to the usenet server
  bool nntp::connect(const std::string& host, const std::string& service)
  {
    if (!socket.connect(host, service))
      return false;
    
    if (read_lines() != 200)
    {
      socket.close();
      return false;
    }
    else
      return true;
  }
  
  // make a secure connection to the usenet server
  bool nntp::secureConnect(const std::string& host, const std::string& service)
  {
    if (!socket.secureConnect(host, service))
      return false;
    
    if (read_lines() != 200)
    {
      socket.close();
      return false;
    }
    else
      return true;
  }
  
  // write a line to the usenet server
  void nntp::write_line(const std::string& line)
  {
    write_line(line.c_str());
  }
  
  // write a line to the usenet server
  void nntp::write_line(const char *line)
  {
    // now write until there is nothing left
    while (strlen(line) > 0)
      line    +=  socket.write_some(line);
  }
  
  int nntp::read_lines(std::string& output)
  {
    // 8kb buffer
    char buf[8192];
    
    // read socket
    int length        =  socket.read_some(buf, 8192);
    
    // remove last two characters from input which are \r\n
    buf[length-2]     = '\0';
    
    // update output
    output = buf;
    
    // return length read
    return atoi(buf);
    
  }
  
  int nntp::read_lines()
  {
    // create returned output
    std::string output;
    
    // read lines
    return read_lines(output);
  }
  
  // write a line to the server and return the response code
  int nntp::process_command(const std::string& line)
  {
    // send the command to the server
    write_line(line);
    
    // and return the result
    return read_lines();
  }
  
  // write a line to the server and return the response code
  int nntp::process_command(const std::string& line, std::string& result)
  {
    // send the command to the server
    write_line(line);
    
    // and return the result
    return read_lines(result);
  }
  
  // write a line to the server and return the response code
  int nntp::process_command(const std::string &line, const int code, std::string &result)
  {
    // send the command to the server
    write_line(line);
    
    // read lines
    int c = read_lines(result);
    
    // throw exception if mismatched
    if (c != code)
      throw decode_exception("Unexpected return code");
    
    return c;
    
  }
  
  // login to the usenet server
  bool nntp::login(const std::string& user, const std::string& pass)
  {
    return process_command("AUTHINFO USER "+user+"\n") == 381 && process_command("AUTHINFO PASS "+pass+"\n") == 281;
  }
  
  // get a usenet group
  group_ptr nntp::open_group(const std::string& name)
  {
    // see if the group exists
    if (process_command("GROUP "+name+"\n") == 211)
    {
      char    *save_ptr;  // pointer used by strtok_r
      long    low;        // low water mark
      long    high;       // high water mark
      
      // not interested in the first part of the line (the estimated number of articles)
      strtok_r(buffer + 4, " ", &save_ptr);
      
      // second and third part are low and high water mark
      low             =   atol(strtok_r(NULL, " ", &save_ptr));
      high            =   atol(strtok_r(NULL, " ", &save_ptr));
      
      // construct the new group
      current_group   =   new group(name, this, low, high);
      
      return current_group;
    }
    
    // group does not exist
    return group_ptr(NULL);
  }
  
  // make sure a group is active on the connection
  void nntp::activate_group(group_ptr open_group)
  {
    // check if we are already the current group
    if (current_group == open_group)
      return;
    
    // send a command to the server to activate this group
    process_command("GROUP "+open_group->name()+"\n");
    
    // and mark it as active
    current_group   =   open_group;
  }
  
  // get the download speed in bytes per second on this connection
  std::size_t nntp::download_speed()
  {
    return socket.download_speed();
  }
  
  // get the upload speed in bytes per second on this connection
  std::size_t nntp::upload_speed()
  {
    return socket.upload_speed();
  }
  
  // disconnect from the usenet server
  void nntp::disconnect()
  {
    // tell the server we are disconnecting
    process_command("QUIT\n");
    
    // and close the connection
    socket.close();
  }
}
