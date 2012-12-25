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

#ifndef INTRUSIVE_PTR_H
#define INTRUSIVE_PTR_H 1

namespace boost
{
    template <typename T> inline void intrusive_ptr_add_ref(T *p)
    {
        // increment reference count of object *p
        ++(p->references);
    }

    template <typename T> inline void intrusive_ptr_release(T *p)
    {
        // decrement reference count
        --(p->references);

        // delete p when no more references exist
        if (p->references == 0)
            delete p;
    } 
}

#endif /* INTRUSIVE_PTR_H */
