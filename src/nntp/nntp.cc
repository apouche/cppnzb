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
    
    if (read_line() != 200)
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
    
    if (read_line() != 200)
    {
      socket.close();
      return false;
    }
    else
      return true;
  }
  
  // read a line from the usenet server and return the status code
  int nntp::read_line()
  {
    boost::system::error_code   error;              // store errors thrown while reading data
    int                         return_code;        // store return code
    int                         length;             // current buffer length
    
    // if we do have some data in the buffer, move it to the beginning
    if (position > buffer)
      memmove(buffer, position, strlen(position) + 1);
    
    // initialize length variable
    length          =   strlen(buffer);
    
    // keep reading until we find a return character in the buffer
    while ((position = strchr(buffer, '\r')) == NULL) {
      length          +=  socket.read_some(buffer + length, sizeof(buffer) - length - 1);
      buffer[length]  =   '\0';
    }
    
    // now split the string at the return character
    *position       =   '\0';
    
    // extract the return code from the line
    return_code     =   atoi(buffer);
    
    // leftovers are after the return character
    position        +=  2;
    
    // and return the result
    return return_code;
  }
  
  // read a line from the usenet server and put it in line
  int nntp::read_line(char *line)
  {
    int result;             // store return code
    
    // do the actual read_line()
    result  =   read_line();
    
    // and store teh contents of the buffer into the provided line
    strncpy(line, buffer + 4, 511);
    
    // and return the result
    return result;
  }
  
  int nntp::read_all_lines(std::string& output)
  {
    // initialize length variable
    char t[1024*8];
    char* buf = t;
    int length          = 0;
    
    length          +=  socket.read_some(buf, 8*1024);
    buf[length] = '\0';
    output = buf;
    return output.length();
    
  }
  
  
  // read a line from the usenet server and put it in line
  int nntp::read_line(std::string& line)
  {
    int result;             // store return code
    
    // do the actual read_line()
    result  =   read_line();
    
    // and store teh contents of the buffer into the provided line
    line    =   buffer + 4;
    
    // and return the result
    return result;
  }
  
  
  
  // read a line from a multiline command and put it in the buffer
  bool nntp::read_multiline()
  {
    boost::system::error_code   error;              // store errors thrown while reading data
    int                         length;             // current buffer length
    
    // if we do have some data in the buffer, move it to the beginning
    if (position > buffer)
      memmove(buffer, position, strlen(position) + 1);
    
    // initialize length variable
    length          =   strlen(buffer);
    
    // keep reading until we find a return character in the buffer
    while ((position = strchr(buffer, '\r')) == NULL) {
      length          +=  socket.read_some(buffer + length, sizeof(buffer) - length - 1);
      buffer[length]  =   '\0';
    }
    
    // now split the string at the return character
    *position       =   '\0';
    position        +=  2;
    
    // single dot, end of results
    if (buffer[0] == '.' && buffer[1] == '\0')
      return false;
    else
      return true;
  }
  
  // read a line from a multiline command and put it in line
  bool nntp::read_multiline(std::string& line)
  {
    // can we still read a line?
    if (!read_multiline())
      return false;
    
    // success, copy the line from the buffer
    line    =   buffer;
    return true;
  }
  
  // read a line from a multiline command and put it in line
  bool nntp::read_multiline(char *line)
  {
    // can we still read a line?
    if (!read_multiline())
      return false;
    
    // success, copy the line from the buffer
    strncpy(line, buffer, 1023);
    return true;
  }
  
  // read a data block from the server
  void nntp::read_block()
  {
    int length;     // current buffer length
    int offset(0);  // offset to start string search
    
    // initialize length variable
    length  =   strlen(position);
    
    // if we do have some data in the buffer, move it to the beginning
    if (position > buffer)
      memmove(buffer, position, length + 1);
    
    // keep reading until we find a the end of the block
    while ((position = strstr(buffer + offset, "\r\n.\r\n")) == NULL) {
      
      // mark the position up to where we searched
      offset          =   length - 4;
      
      // offset cannot be negative
      if (offset < 0)
        offset = 0;
      
      // and read the next batch
      length          +=  socket.read_some(buffer + length, sizeof(buffer) - length - 1);
      buffer[length]  =   '\0';
    }
    
    // split the string
    *position   =   '\0';
    position    +=  5;
  }
  
  // read a data block from the server
  void nntp::read_block(std::string& block)
  {
    // read the block into the buffer
    read_block();
    
    // put the data in the block
    block   =   buffer;
  }
  
  // read a data block from the server
  void nntp::read_block(char *block)
  {
    // read the block into the buffer
    read_block();
    
    // put the data in the block
    strncpy(block, buffer, 1048575);
  }
  
  // read a data block from the server
  int nntp::read_block(char **block)
  {
    int length;             // length of return data
    
    // read the block into the buffer
    read_block();
    
    // allocate a buffer to hold it
    length  =   strlen(buffer);
    *block  =   new char [length + 1];
    
    // copy the buffer contents to the new block
    strcpy(*block, buffer);
    
    // and return the length
    return length;
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
  
  // write a line to the server and return the response code
  int nntp::process_command(const std::string& line)
  {
    // send the command to the server
    write_line(line);
    
    // and return the result
    return read_line();
  }
  
  // write a line to the server and return the response code
  int nntp::process_command(const char *line, char *result)
  {
    // send the command to the server
    write_line(line);
    
    // and return the result
    return read_line(result);
  }
  
  // write a line to the server and return the response code
  int nntp::process_command(const std::string& line, std::string& result)
  {
    // send the command to the server
    write_line(line);
    
    // and return the result
    return read_line(result);
  }
  
  // login to the usenet server
  bool nntp::login(const std::string& user, const std::string& pass)
  {
    return process_command("AUTHINFO USER "+user+"\n") == 381 && process_command("AUTHINFO PASS "+pass+"\n") == 281;
  }
  
  // write a line to the server and process it's multiline response
  int nntp::process_block_command(const char *line, int code, char **result)
  {
    // check if the return code from the server is correct
    if (code >= 0 && process_command(line) != code)
      throw server_exception("Unexpected reply from server.");
    
    // read the multiline response
    return read_block(result);
  }
  
  int nntp::process_block_string(const std::string& str, std::string& output)
  {
    //    std::string t = str + "\n";
    
    //    std::cout << t << std::endl;
    
    
    write_line(str);
    
    return read_all_lines(output);
    
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
