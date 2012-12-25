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

#ifndef DECODED_ARTICLE_H
#define DECODED_ARTICLE_H 1

#include <string>
#include <fstream>
#include <boost/intrusive_ptr.hpp>
#include "intrusive_ptr.h"
#include "exceptions.h"

namespace nntp
{
    /**
      * @class nntp::decoded_article
      *
      * This class provides functionality to work with encoded article data.
      */
    class decoded_article
    {
        private:
            long        part;       // part number
            long        parts;      // number of parts
            long        part_size;  // size of this part
            long        part_begin; // start of part in full file
            long        part_end;   // end of part in full file
            long        size;       // total size of the file
            std::string content;    // decoded contents
            std::string orig_name;  // pointer to original filename
            std::size_t references; // reference count to this object

            friend void ::boost::intrusive_ptr_add_ref<>(decoded_article *p);
            friend void ::boost::intrusive_ptr_release<>(decoded_article *p);

            long        read_param(const char *param, int length, const char *line_begin);
            const char  *parse_header(const char *source);
            const char  *parse_footer(const char *source);
            const char  *decode(const char *data, std::size_t expected);
        public:
            /**
              * Construct based on undecoded source and length
              *
              * @throws decode_exception
              *
              * @param  source  pointer to array with source
              * @param  length  size of source array
              */
            decoded_article(const char *source, int length);

            /**
              * Destructor
              */
            ~decoded_article();

            /**
              * Is this a multipart binary
              *
              * @return whether this is a multipart binary
              */
            bool multipart();

            /**
              * What is the part number of this file?
              *
              * @return the part number
              */
            long part_number();

            /**
              * At what position in the file does this part begin?
              *
              * @return position of first byte in the file
              */
            long begin();

            /**
              * Retrieve the decoded data
              *
              * @return contents    the decoded data
              */
            const std::string& data();

            /**
              * Put the filename in the provided string
              *
              * @param  name        string to put the name in
              * @return whether the filename was successfully put in the string
              */
            const std::string& filename();
    };
}

#endif /* DECODED_ARTICLE_H */
