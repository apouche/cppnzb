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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H 1

#include <stdexcept>
#include <string>

namespace nntp
{
    class network_exception : std::runtime_error { public: network_exception(   const std::string& what_arg) : runtime_error(what_arg) {} };
    class server_exception  : std::runtime_error { public: server_exception(    const std::string& what_arg) : runtime_error(what_arg) {} };
    class decode_exception  : std::runtime_error { public: decode_exception(    const std::string& what_arg) : runtime_error(what_arg) {} };
}

#endif /* EXCEPTIONS_H */
