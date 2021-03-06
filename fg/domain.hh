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

#ifndef __TFBAYES_FG_DOMAIN_HH__
#define __TFBAYES_FG_DOMAIN_HH__

#ifdef HAVE_CONFIG_H
#include <fg/config.h>
#endif /* HAVE_CONFIG_H */

#include <numeric>
#include <cmath>
#include <cstddef>

#include <fg/clonable.hh>

#include <boost/icl/interval_set.hpp>

class domain_i : public virtual clonable {
public:
        virtual ~domain_i() { }

        virtual domain_i* clone() const = 0;

        virtual bool element(const std::vector<double>& x) const = 0;
};

class real_domain_t : public domain_i {
public:
        typedef boost::icl::interval<double> interval_t;
        typedef std::numeric_limits<double> limits_t;

        real_domain_t(size_t n) :
                interval(interval_t::open(-limits_t::infinity(), limits_t::infinity())),
                n       (n)
                { }
        real_domain_t(size_t n, const interval_t::type& interval) :
                interval(interval),
                n       (n)
                { }
        real_domain_t(const real_domain_t& real_domain) :
                interval(real_domain.interval),
                n       (real_domain.n)
                { }

        virtual real_domain_t* clone() const {
                return new real_domain_t(*this);
        }

        virtual bool element(const std::vector<double>& x) const {
                if (x.size() != n) {
                        return false;
                }
                for (size_t i = 0; i < x.size(); i++) {
                        if (!boost::icl::contains(interval, x[i])) {
                                return false;
                        }
                }
                return true;
        }
protected:
        interval_t::type interval;
        size_t n;
};

inline
real_domain_t positive_domain_t(size_t n) {
        return real_domain_t(n,
                             boost::icl::interval<double>::open(
                                     0,
                                     std::numeric_limits<double>::infinity()));
}

class discrete_domain_t : public domain_i {
public:
        discrete_domain_t(size_t n) :
                n       (n)
                { }
        discrete_domain_t(const discrete_domain_t& discrete_domain) :
                n       (discrete_domain.n)
                { }

        virtual discrete_domain_t* clone() const {
                return new discrete_domain_t(*this);
        }

        virtual bool element(const std::vector<double>& x) const {
                double tmp;
                if (x.size() != n) {
                        return false;
                }
                for (size_t i = 0; i < x.size(); i++) {
                        if (std::modf(x[i], &tmp) != 0.0) {
                                return false;
                        }
                }
                return true;
        }
protected:
        size_t n;
};

class simplex_t : public domain_i {
public:
        simplex_t(size_t n) :
                n(n)
                { }
        simplex_t(const simplex_t& simplex) :
                n(simplex.n)
                { }

        virtual simplex_t* clone() const {
                return new simplex_t(*this);
        }

        virtual bool element(const std::vector<double>& x) const {
                if (x.size() != n) {
                        return false;
                }
                double sum = std::accumulate(x.begin(), x.end(), 0.0);
                if (std::abs(sum - 1.0) > 1.0e-10) {
                        return false;
                }
                return true;
        }
protected:
        size_t n;
};

#endif /* __TFBAYES_FG_DOMAIN_HH__ */
