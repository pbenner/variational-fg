/* Copyright (C) 2010-2013 Philipp Benner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TFBAYES_UTILITY_LINALG_HH__
#define __TFBAYES_UTILITY_LINALG_HH__

#ifdef HAVE_CONFIG_H
#include <fg/config.h>
#endif /* HAVE_CONFIG_H */

#include <stddef.h>
#include <vector>

/* since this is the only position where c++0x might be useful
 * in the code we try to stick with boost here */
//#include <type_traits>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

// c++ matrix
////////////////////////////////////////////////////////////////////////////////

namespace std {
        template <typename T>
        class matrix : public vector<vector<T> > {
        public:
                matrix()
                        : vector<vector<T> >()
                        {}
                matrix(size_t rows, size_t columns)
                        : vector<vector<T> >(rows, vector<T>(columns, 0.0))
                        {}
                matrix(size_t rows, size_t columns, T init)
                        : vector<vector<T> >(rows, vector<T>(columns, init))
                        {}
                template <typename InputIterator>
                matrix (InputIterator first,
                        InputIterator last,
                        const allocator<vector<double> >& a = allocator<vector<double> >(),
                        typename boost::disable_if<boost::is_integral<InputIterator> >::type* = 0)
                        : vector<vector<T> >(first, last, a) { }
                matrix<T> transpose(T init = 0.0) {
                        matrix<T> m(columns(), rows(), init);
                        for (size_t i = 0; i < columns(); i++) {
                                for (size_t j = 0; j < rows(); j++) {
                                        m[i][j] = (*this)[j][i]; 
                                }
                        }
                        return m;
                }
                size_t rows() const {
                        return matrix<T>::size();
                }
                size_t columns() const {
                        return rows() == 0 ? 0 : operator[](0).size();
                }
                using vector<vector<T> >::operator[];
        };
}

#endif /* __TFBAYES_UTILITY_LINALG_HH__ */
