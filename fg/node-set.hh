/* Copyright (C) 2013 Philipp Benner
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

#ifndef __TFBAYES_FG_NODE_SET_HH__
#define __TFBAYES_FG_NODE_SET_HH__

#ifdef HAVE_CONFIG_H
#include <fg/config.h>
#endif /* HAVE_CONFIG_H */

#include <map>
#include <string>

#include <boost/range.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <fg/node-types.hh>
#include <fg/default-operator.hh>

template<typename T>
T& dereference(T* ref) {
        return *ref;
}

template <class T>
class node_set_t : private boost::ptr_vector<T> {
public:
        // type of the base class
        typedef boost::ptr_vector<T> base_t;
        typedef std::map<std::string, std::vector<T*> > map_t;

        typedef boost::transform_iterator<
                const T& (*)(const T*),
                typename std::vector<T*>::const_iterator
                > const_transform_iterator;
        typedef boost::transform_iterator<
                T& (*)(T*),
                typename std::vector<T*>::iterator
                > transform_iterator;

        typedef boost::iterator_range<const_transform_iterator> const_range;
        typedef boost::iterator_range<      transform_iterator>       range;

        // // access values
        range find(const std::string& name) {
                typename map_t::iterator it = map.find(name);
                if (it != map.end()) {
                        return boost::make_iterator_range(
                                boost::make_transform_iterator(it->second.begin(), dereference<T>),
                                boost::make_transform_iterator(it->second.end  (), dereference<T>));
                }
                return range();
        }
        const_range find(const std::string& name) const {
                typename map_t::const_iterator it = map.find(name);
                if (it != map.end()) {
                        return boost::make_iterator_range(
                                boost::make_transform_iterator(it->second.begin(), dereference<const T>),
                                boost::make_transform_iterator(it->second.end  (), dereference<const T>));
                }
                return const_range();
        }
        node_set_t<T>& operator+=(T* node) {
                map[node->name()].push_back(node);
                base_t::push_back(node);
                return *this;
        }
        using base_t::begin;
        using base_t::cbegin;
        using base_t::end;
        using base_t::cend;
        using typename base_t::iterator;
        using typename base_t::const_iterator;
protected:
        map_t map;
};


typedef node_set_t<factor_node_i> factor_set_t;
typedef node_set_t<variable_node_i> variable_set_t;

#endif /* __TFBAYES_FG_NODE_SET_HH__ */
