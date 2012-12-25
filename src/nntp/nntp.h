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
    int     read_lines(std::string& output);
    
    /**
     * Read the status code from the usenet server
     *
     * @note   The actual text sent back by the server (without the trailing
     *         newline) is in the buffer variable.
     * @return status code returned by the server
     */
    int     read_lines();
    
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
     * @note   This is just a shortcut to call
     *         write_line() and then read_line()
     *
     * @param  line    string to write to the server
     * @param  result  string to write the result to
     * @return return code from the server
     */
    int     process_command(const std::string& line, std::string& result);
    
    /**
     * Send a command and return the reply status
     *
     * @note   This is just a shortcut to call
     *         write_line() and then read_line()
     *
     * @param  line    string to write to the server
     * @param  result  string to write the result to
     * @param  code    integer holding the expected result code
     * @return return code from the server
     */
    int     process_command(const std::string& line, const int code, std::string& result);
    
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
