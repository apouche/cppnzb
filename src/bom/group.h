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

#ifndef GROUP_H
#define GROUP_H 1

#include "intrusive_ptr.h"
#include "nntp.h"

namespace nntp
{
    // forward declarations
    class article;

    // typedefs
    typedef boost::intrusive_ptr<article>  article_ptr;

    /**
      * @class  nntp::group
      *
      * This class represents a single usenet group. Articles can be retrieved from the group by using the fetch_article()
      * function. See the nntp::article class description for more information on how to work with articles.
      */
    class group
    {
        private:
            class nntp                  *connection;        // usenet connection
            long                        low;                // low water mark in group
            long                        high;               // high water mark in group
            std::string                 group_name;         // name of the group
            size_t                      references;         // reference count to this object

            friend void ::boost::intrusive_ptr_add_ref<>(group *p);
            friend void ::boost::intrusive_ptr_release<>(group *p);
        public:
            /**
              * Construct group based on low and high water mark
              *
              * @param  name        the name of the group
              * @param  connection  connection to our usenet server
              * @param  low         low water mark
              * @param  high        high water mark
              */
            group(const std::string& name, nntp *connection, long low, long high);

            /**
              * Destructor
              */
            ~group();

            /**
              * Make sure group is active
              */
            void activate();

            /**
              * Get the group name
              *
              * @return name of the group
              */
            const std::string& name();

            /**
              * Fetch an article from the group
              *
              * @param  number      article number in group
              */
            article_ptr fetch_article(long number);

            /**
              * Fetch an article from the group
              *
              * @param  msg_id      message id
              */
            article_ptr fetch_article(const std::string& msg_id);
    };
}

#include "article.h"
#endif /* GROUP_H */
