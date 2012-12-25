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

#ifndef NNTP_H
#define NNTP_H 1


#include "intrusive_ptr.h"
#include "socket_wrapper.h"

namespace nntp
{
  // forward declarations
  class group;
  
  // typedefs
  typedef boost::intrusive_ptr<group>    group_ptr;
  
  /**
   * @class  nntp::nntp
   *
   * This class provides a single connection to a usenet server. See the connect() and login() functions. Once connected, groups can be
   * accessed by calling the open_group() function. See the nntp::group class description for more information on how to work with groups.
   */
  class nntp
  {
  private:
    socket_wrapper  socket;         // socket connection to usenet server
    group_ptr       current_group;  // pointer to currently active group
    char            buffer[1048576];// buffer for incoming data (15 K)
    char            *position;      // current position in buffer
    
    void initialize();
  public:
    /**
     * Default constructor
     */
    nntp();
    
    /**
     * Destructor
     */
    ~nntp();
    
    /**
     * Make a connection to the usenet server
     *
     * @param  host    hostname
     * @param  service service name or port
     * @return connected or not
     */
    bool    connect(const std::string& host, const std::string& service = "nntp");
    
    /**
     * Make a secure connection to the usenet server
     *
     * @param  host    hostname
     * @param  service service name or port
     * @param  service service name
     */
    bool    secureConnect(const std::string& host, const std::string& service = "nntps");
    
    /**
     * Read the status code from the usenet server
     *
     * @note   The actual text sent back by the server (without the trailing
     *         newline) is in the buffer variable.
     * @return status code returned by the server
     */
    int     read_line();
    
    int     read_all_lines(std::string& output);
    
    /**
     * Read a line from the usenet server and return the status code
     *
     * @note   The trailing enter is removed from the line. The line
     *         buffer should be at least 512 bytes long.
     *
     * @param  line    string to put the line in
     * @return status code returned by the server
     */
    int     read_line(char *line);
    
    /**
     * Read a line from the usenet server and return the status code
     *
     * @note   The trailing enter is removed from the line.
     *
     * @param  line    string to put the line in
     * @return status code returned by the server
     */
    int     read_line(std::string& line);
    
    /**
     * Read one line from a multiline response. Returns false when
     * all lines have been read.
     *
     * @note   The actual line can be read from the buffer variable.
     *
     * @return was a line read from the server?
     */
    bool    read_multiline();
    
    /**
     * Read one line from a multiline response. Returns false when
     * all lines have been read.
     *
     * @note   Removes extra leading dots from the result
     *
     * @param  line    string to put line in
     * @return was a line read from the server?
     */
    bool    read_multiline(std::string& line);
    
    /**
     * Read one line from a multiline response. Returns false when
     * all lines have been read.
     *
     * @note   Removes extra leading dots from the result. The input
     *         buffer should be at least 1024 characters.
     *
     * @param  line    string to put line in
     * @return was a line read from the server?
     */
    bool    read_multiline(char *line);
    
    /**
     * Read an entire multiline response and put it in the buffer.
     *
     * @note   Extra leading dots are *NOT* removed from the result.
     */
    void    read_block();
    
    /**
     * Read an entire multiline response
     *
     * @note   Extra leading dots are *NOT* removed from the result.
     *
     * @param  block   string to put the block in
     */
    void    read_block(std::string& block);
    
    /**
     * Read an entire multiline response
     *
     * @note   Extra leading dots are *NOT* removed from the result.
     *         The input buffer should be at least 1048576 characters.
     *
     * @param  block   string to put the block in
     */
    void    read_block(char *block);
    
    /**
     * Read an entire multiline response and put the buffer location
     * in the character pointer
     *
     * @note   The pointer has to be deallocated by hand.
     *
     * @param  block   pointer to pointer which will contain the returned
     *                 data from the server
     * @return number of characters in block
     */
    int     read_block(char **block);
    
    /**
     * Write a line to the usenet server
     *
     * @param  line    line to send
     */
    void    write_line(const std::string& line);
    
    /**
     * Write a line to the usenet server
     *
     * @param  line    line to send
     */
    void    write_line(const char *line);
    
    /**
     * Send a command and return the reply status
     *
     * @note   This is just a shortcut to call
     *         write_line() and then read_line()
     *
     * @param  line    string to write to the server
     * @return return code from the server
     */
    int     process_command(const std::string& line);
    
    /**
     * Send a command and return the reply status
     *
     * @note   The result buffer should be at least
     *         512 bytes. This is just a shortcut to
     *         call write_line() and then read_line()
     *
     * @param  line    string to write to the server
     * @param  result  string to write the result to
     * @return return code from the server
     */
    int     process_command(const char *line, char *result);
    
    /**
     * Send a command and return the reply status
     *
     * @note   This is just a shortcut to call
     *         write_line() and then read_line()
     *
     * @param  line    string to write to the server
     * @param  result  string to write the result to
     * @return return code from the server
     */
    int     process_command(const std::string& line, std::string& result);
    
    /**
     * Send a command to the server and retrieve it's
     * multiline response
     *
     * @note   The returned result should be freeed when
     *         it is no longer used
     *
     * @throws network_exception, server_exception
     *
     * @param  line    command to send
     * @param  code    status code expected
     * @param  result  pointer to pointer where the result is written to
     * @result number of bytes in buffer
     */
    int     process_block_command(const char *line, int code, char **result);
    
    int     process_block_string(const std::string& str, std::string& output);
    
    /**
     * Login to the usenet server
     *
     * @param  user    username
     * @param  pass    password
     * @return successfully connected and logged in
     */
    bool    login(const std::string& user, const std::string& pass);
    
    /**
     * Return a pointer to a usenet group
     *
     * @note   Returns NULL if the group does not exist
     *         or if it is inaccessible
     *
     * @param  name    name of the usenet group to retrieve
     */
    group_ptr   open_group(const std::string& name);
    
    /**
     * Make sure a group is active on the nntp connection
     *
     * @param  open_group  the group to activate
     */
    void    activate_group(group_ptr open_group);
    
    /**
     * Get the download speed in bytes per second
     *
     * @return the number of bytes per second
     */
    std::size_t download_speed();
    
    /**
     * Get the upload speed in bytes per second
     *
     * @return the number of bytes per second
     */
    std::size_t upload_speed();
    
    /**
     * Disconnect from the usenet server
     *
     * @note   The only reason this might fail is if there is no
     *         open connection to the usenet server.
     */
    void    disconnect();
  };
}

#include "group.h"
#endif /* NNTP_H */
