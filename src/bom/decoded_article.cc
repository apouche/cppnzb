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



#include "decoded_article.h"

namespace nntp
{
    // read a long parameter from the provided line
    long decoded_article::read_param(const char *param, int length, const char *line_begin)
    {
        const char *current;    // pointer to current location in line

        // if we cannot find the parameter just return
        if ((current = strstr(line_begin, param)) == NULL)
            return 0;

        // return the parameter
        return atol(current + length);
    }

    // parse the yend header line and return a pointer to it's end
    const char *decoded_article::parse_header(const char *source)
    {
        const char  *line_begin;    // pointer to beginning of line
        const char  *line_end;      // pointer to end of line
        char        *current;       // pointer to current character
        char        *line;          // pointer to line buffer
        int         line_length;    // length of current line

        // does the source begin straight with =ybegin?
        if (strncmp(source, "=ybegin ", 8) == 0)
            line_begin  =   source;
        // or does it occur further on in the source
        else if ((line_begin = strstr(source, "\r\n=ybegin ")) != NULL)
            line_begin  +=  2;  // skip over the \r\n characters
        // or does it not occur at all
        else
            throw decode_exception("Yend header not found, is this really a yend-encoded article");

        // find the end of the =ybegin line
        if ((line_end = strstr(line_begin, "\r\n")) == NULL)
            throw decode_exception("Yend header line not correctly closed");

        // copy it to a temporary buffer
        line_length =   line_end - line_begin;
        line        =   (char *) alloca(line_length + 1);
        strncpy(line, line_begin, line_length);
        *(line + line_length) = '\0';

        // find the size parameter in the line
        if ((size = read_param(" size=", 6, line)) == 0)
            throw decode_exception("Required parameter 'size' not found in yend header line");

        // find the filename
        if ((current = strstr(line, " name=")) == NULL)
            throw decode_exception("Required parameter 'name' not found in yend header line");

        // copy it to the filename variable
        orig_name   =   current + 6;

        // if it isn't a multipart binary we're done now
        if ((part = read_param(" part=", 6, line)) == 0)
            return line_end;

        // try to read the (optional) total parameter
        parts       =   read_param(" total=", 7, line);

        // skip to the next line
        line_begin  =   line_end + 2;
        line_end    =   strstr(line_begin, "\r\n");

        // find the end of the =ypart line
        if (line_end == NULL)
            throw decode_exception("Ypart line not found");

        // copy it to a temporary buffer
        line_length =   line_end - line_begin;
        line        =   (char *) alloca(line_length + 1);
        strncpy(line, line_begin, line_length);
        *(line + line_length) = '\0';

        // we should have a line end and the line should start with =ypart
        if (strncmp(line, "=ypart ", 7) != 0)
            throw decode_exception("Required ypart line not found");

        // we should have a begin and an end parameter
        part_begin  =   read_param(" begin=", 7, line);
        part_end    =   read_param(" end=", 5, line);
        part_size   =   part_end - part_begin + 1;

        if (part_begin == 0 || part_end == 0)
            throw decode_exception("Required parameter 'name' or 'begin not found in ypart header");

        // if the part size equals the total size, we have a fake multipart
        // binary on our hands! yes, there are people who do this!
        if (size == part_size) {
            part        =   0;
            parts       =   0;
            part_size   =   0;
            part_begin  =   0;
            part_end    =   0;
        }
        // if the parts parameter was not provided, take a guess
        else if (parts == 0)
            parts   =   (size - 1) / part_size + 1;

        // done
        return line_end;
    }

    // parse the yend footer line and return a pointer to it's end
    const char *decoded_article::parse_footer(const char *source)
    {
        return NULL;
    }

    // decode data
    const char *decoded_article::decode(const char *data, std::size_t expected)
    {
        // keep looping until we are at the end of the data
        while (true) {
            // line break
            if (*data == '\r') {
                // do we have to skip over an extra dot?
                if (*(data + 2) == '.')
                    data    +=  3;
                // end of input stream
                else if (strncmp(data + 2, "=yend ", 6) == 0) {
                    data    +=  2;
                    break;
                }
                // normal line break
                else
                    data    +=  2;
            }
            // do we already have enough characters
            if (content.size() == expected)
                // too many characters in input buffer
                throw decode_exception("Too many characters in input buffer");
            // do we have an escape character
            if (*data == '=') {
                ++data;
                content.push_back((*data + 150) % 256);
            }
            // no escape character
            else
                content.push_back((*data + 214) % 256);

            // next character
            ++data;
        }

        if (content.size() != expected)
            // not enough characters in input buffer
            throw decode_exception("Not enough characters in input buffer");
        else
            return data;
    }

    // initialize decoded article given source and its length
    decoded_article::decoded_article(const char *source, int length) :
        size(0),
        references(0)
    {
        const char  *current;       // pointer to current character

        // parse the header, footer, and decode the content
        current =   parse_header(source);
        current =   decode(current, parts > 0 ? part_size : size);
        current =   parse_footer(current);
    }

    // clean up
    decoded_article::~decoded_article()
    {}

    // multipart or not
    bool decoded_article::multipart()
    {
        return parts > 0;
    }

    // part number
    long decoded_article::part_number()
    {
        return part;
    }

    // first byte in the file
    long decoded_article::begin()
    {
        return part_begin;
    }

    // access the decoded data
    const std::string& decoded_article::data()
    {
        return content;
    }

    // give the filename with an optional part suffix
    const std::string& decoded_article::filename()
    {
        return orig_name;
    }
}
