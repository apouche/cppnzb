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

#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H 1

#include <string>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "exceptions.h"

namespace nntp
{
    // typedefs
    typedef boost::asio::ip::tcp::socket        unsecure;
    typedef boost::asio::ssl::stream<unsecure>  secure;
    typedef boost::asio::ssl::context           ssl_context;
    typedef std::queue<std::size_t>             byte_counter;

    /**
      * @class  nntp::socket_wrapper
      *
      * A simple socket abstraction class, allowing the use of a single object for both
      * normal sockets as well as ssl encrypted ones. Use connect() or secureConnect()
      * to set up a connection.
      */
    class socket_wrapper
    {
        private:
            unsecure                    *tcp_sock;  // unsecure socket connection to usenet server
            secure                      *ssl_sock;  // secure socket connection to usenet server
            boost::posix_time::ptime    slice_time; // store time at which to switch to next slice
            boost::asio::io_service     io_service; // io_service required by boost::asio
            byte_counter                incoming;   // store count of incoming bytes over last x seconds
            byte_counter                outgoing;   // store count of outgoing bytes over last x seconds
            std::size_t                 slice_inc;  // store count of incoming bytes this slice
            std::size_t                 slice_out;  // store count of outgoing bytes this slice
            std::size_t                 inc_bytes;  // incoming bytes over the whole measured period
            std::size_t                 out_bytes;  // outgoing bytes over the whole measured period

            /**
              * Switch to the next slice when the time is right
              */
            void update_slices();

            /**
              * Log input and output data
              * @param  received    number of bytes received
              * @param  sent        number of bytes sent
              */
            void log_io(std::size_t received, std::size_t sent);

            /**
              * Clear input and output log, done after connection is broken
              */
            void clear_log();
        public:
            /**
              * Constructor
              */
            socket_wrapper();

            /**
              * Destructor
              */
            ~socket_wrapper();

            /**
              * Make a connection to the usenet server
              *
              * @param  host    hostname
              * @param  service service name or port 
              * @return connected or not
              */
            bool connect(const std::string& host, const std::string& service = "nntp");

            /**
              * Make a connection to the usenet server
              *
              * @param  host    hostname
              * @param  service service name or port 
              * @return connected or not
              */
            bool secureConnect(const std::string& host, const std::string& service = "nntps");

            /**
              * @return whether the socket is connected to an endpoint
              */
            bool is_open();

            /**
              * Close the connection
              */
            void close();

            /**
              * Read data into a buffer
              *
              * @throws network_exception
              *
              * @param  buffer  buffer to read data into
              * @param  length  maximum length to read
              * @return number of bytes read
              */
            std::size_t read_some(char *buffer, std::size_t length);

            /**
              * Write data from a buffer
              *
              * @throws network_exception
              *
              * @param  buffer  buffer to read data from
              * @return number of bytes written
              */
            std::size_t write_some(const char *buffer);

            /**
              * Get the incoming transfer speed in bytes per second
              * @return the amount of bytes coming in per second
              */
            std::size_t download_speed();

            /**
              * Get the outgoing transfer speed in bytes per second
              * @return the amount of bytes coming in per second
              */
            std::size_t upload_speed();
    };
}

#endif /* SOCKET_WRAPPER_H */
