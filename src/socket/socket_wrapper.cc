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

#include "socket_wrapper.h"

namespace nntp
{
    // constructor
    socket_wrapper::socket_wrapper() :
        tcp_sock(NULL),
        ssl_sock(NULL),
        inc_bytes(0),
        out_bytes(0)
    {}

    // constructor
    socket_wrapper::~socket_wrapper()
    {
        close();
    }

    // update slices
    void socket_wrapper::update_slices()
    {
        // check if a tenth of a second has passed since the last switch
        while (boost::posix_time::microsec_clock::universal_time() >= slice_time)
        {
            // add the current slices to the totals
            inc_bytes   +=  slice_inc;
            out_bytes   +=  slice_out;

            // do we already have the required amount of slices
            if (incoming.size() == 30)
            {
                // subtract the old amount from the totals
                inc_bytes   -=  incoming.front();
                out_bytes   -=  outgoing.front();

                // and remove it from the queue
                incoming.pop();
                outgoing.pop();
            }

            // push them into our queue
            incoming.push(slice_inc);
            outgoing.push(slice_out);

            // and make them empty for the next iteration
            slice_inc   =   0;
            slice_out   =   0;

            // set the time for the next iteration
            slice_time  +=  boost::posix_time::millisec(100);
        }
    }

    // log input and output data
    void socket_wrapper::log_io(std::size_t received, std::size_t sent)
    {
        // make sure slices are accurate
        update_slices();

        // log the data
        slice_inc   +=  received;
        slice_out   +=  sent;
    }

    // clear the io log
    void socket_wrapper::clear_log()
    {
        while (incoming.size() > 0)
        {
            // pop all items from the log 
            incoming.pop();
            outgoing.pop();
        }

        // clear the current slice
        slice_inc   =   0;
        slice_out   =   0;

        // and clear the total byte count
        inc_bytes   =   0;
        out_bytes   =   0;
    }

    // make a connection to the usenet server
    bool socket_wrapper::connect(const std::string& host, const std::string& service)
    {
        boost::asio::ip::tcp::resolver::iterator    endpoint_iterator;      // iterator to iterate over endpoints
        boost::asio::ip::tcp::resolver::iterator    endpoint;               // uninitialized endpoint 
        boost::asio::ip::tcp::resolver::query       query(host, service);   // query to resolve into endpoints
        boost::asio::ip::tcp::resolver              resolver(io_service);   // resolver to resolve the query
        boost::system::error_code                   error;                  // error returned by boost

        // cannot proceed if already connected
        if (tcp_sock != NULL || ssl_sock != NULL)
            return false;

        // try to resolve to an endpoint
        endpoint_iterator   =   resolver.resolve(query, error);

        // see if we received an error
        if (error)
            return false;

        // allocate a new socket
        tcp_sock    =   new unsecure(io_service);

        // loop through the available endpoints until we can connect without an error
        while (endpoint_iterator != endpoint) {
            // try to connect to the endpoint
            if (!tcp_sock->connect(*endpoint_iterator, error))
            {
                // no data has been transmitted yet
                incoming.push(0);
                outgoing.push(0);

                // store the current time + a tenth of a second
                slice_time  =   boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(100);

                // all done
                return true;
            }

            // and close the socket again
            tcp_sock->close();

            // increment the iterator
            ++endpoint_iterator;
        }

        // we got through the loop without getting a connection
        delete tcp_sock;
        tcp_sock    =   NULL;

        // we did our best
        return false;
    }

    // make a connection to the usenet server
    bool socket_wrapper::secureConnect(const std::string& host, const std::string& service)
    {
        boost::asio::ip::tcp::resolver::iterator    endpoint_iterator;                  // iterator to iterate over endpoints
        boost::asio::ip::tcp::resolver::iterator    endpoint;                           // uninitialized endpoint 
        boost::asio::ip::tcp::resolver::query       query(host, service);               // query to resolve into endpoints
        boost::asio::ip::tcp::resolver              resolver(io_service);               // resolver to resolve the query
        boost::asio::ssl::context                   context(io_service, ssl_context::sslv23);   // boost ssl context
        boost::system::error_code                   error;                              // error returned by boost

        // cannot proceed if already connected
        if (tcp_sock != NULL || ssl_sock != NULL)
            return false;

        // try to resolve to an endpoint
        endpoint_iterator   =   resolver.resolve(query, error);

        // see if we received an error
        if (error)
            return false;

        // set ssl options and allocate a new socket
        // context.set_verify_mode(ssl_context::verify_peer);
        // context.load_verify_file("/etc/ssl/certs/ca.pem");
        ssl_sock    =   new secure(io_service, context);

        // loop through the available endpoints until we can connect without an error
        while (endpoint_iterator != endpoint) {
            // try to connect to the endpoint and do a handshake
            if (!ssl_sock->lowest_layer().connect(*endpoint_iterator, error) && !ssl_sock->handshake(boost::asio::ssl::stream_base::client, error))
            {
                // no data has been transmitted yet
                incoming.push(0);
                outgoing.push(0);

                // store the current time + a tenth of a second
                slice_time  =   boost::posix_time::microsec_clock::universal_time() + boost::posix_time::millisec(1000000);

                // all done
                return true;
            }

            // and close the socket again
            ssl_sock->lowest_layer().close();

            // increment the iterator
            ++endpoint_iterator;
        }

        // we got through the loop without getting a connection
        delete ssl_sock;
        ssl_sock    =   NULL;

        // we did our best
        return false;
    }

    // check if the socket is connected to an endpoint
    bool socket_wrapper::is_open()
    {
        // check whether we have a socket and if it is open
        return ((tcp_sock != NULL && tcp_sock->is_open())
               || (ssl_sock != NULL && ssl_sock->lowest_layer().is_open()));
    }

    // close the connection
    void socket_wrapper::close()
    {
        // do we have a normal connection
        if (tcp_sock != NULL)
        {
            // close, delete and set to null
            tcp_sock->close();
            delete tcp_sock;
            tcp_sock    =   NULL;
        }
        // or a secured one
        else if (ssl_sock != NULL)
        {
            // close, delete and set to null
            ssl_sock->lowest_layer().close();
            delete ssl_sock;
            ssl_sock    =   NULL;
        }

        // clear the io log
        clear_log();
    }

    // read some data into a buffer
    std::size_t socket_wrapper::read_some(char *buffer, std::size_t length)
    {
        boost::system::error_code   error;  // error returned by boost
        std::size_t                 bytes;  // number of bytes read

        // if we do not have a connection, we cannot read from the socket
        if (!is_open())
            throw network_exception("Unable to read from non-connected socket.");

        // do we have an unsecured socket?
        if (tcp_sock != NULL)
            // read data
            bytes   =   tcp_sock->read_some(boost::asio::buffer(buffer, length), error);
        // or an unsecured one
        else
            // read data
            bytes   =   ssl_sock->read_some(boost::asio::buffer(buffer, length), error);

        // check if we received an error
        if (error)
        {
            // mark the socket closed on our end too
            close();

            // raise an exception to indicate something went wrong
            throw network_exception("The network connection was unexpectedly closed.");
        }

        // log it
        log_io(bytes, 0);

        // and return the result
        return bytes;
    }

    // write some data from a buffer
    std::size_t socket_wrapper::write_some(const char *buffer)
    {
        boost::system::error_code   error;  // error returned by boost
        std::size_t                 bytes;  // number of bytes written

        // if we do not have a connection, we cannot wrote to the socket
        if (!is_open())
            throw network_exception("Unable to write to non-connected socket.");

        // do we have an unsecured socket?
        if (tcp_sock != NULL)
            // write data
            bytes   =   tcp_sock->write_some(boost::asio::buffer(buffer,strlen(buffer)), error);
        // or an unsecured one
        else
            // write data
            bytes   =   ssl_sock->write_some(boost::asio::buffer(buffer,strlen(buffer)), error);

        // check if we received an error
        if (error)
        {
            // mark the socket closed on our end too
            close();

            // raise an exception to indicate something went wrong
            throw network_exception("The network connection was unexpectedly closed.");
        }

        // log it
        log_io(0, bytes);

        // and return the result
        return bytes;
    }

    // get incoming bytes per second
    std::size_t socket_wrapper::download_speed()
    {
        // update slices in case there hasn't been data for a while
        update_slices();

        // and calculate the speed
        return inc_bytes * 10 / incoming.size();
    }

    // get outgoing bytes per second
    std::size_t socket_wrapper::upload_speed()
    {
        // update slices in case there hasn't been data for a while
        update_slices();

        // and calculate the speed
        return out_bytes * 10 / outgoing.size();
    }
}
