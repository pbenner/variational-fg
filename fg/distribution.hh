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

#ifndef __TFBAYES_FG_DISTRIBUTION_HH__
#define __TFBAYES_FG_DISTRIBUTION_HH__

#ifdef HAVE_CONFIG_H
#include <fg/config.h>
#endif /* HAVE_CONFIG_H */

#include <algorithm>    // std::transform
#include <functional>   // std::plus
#include <limits>
#include <numeric>
#define _USE_MATH_DEFINES
#include <cmath>

#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/digamma.hpp>

#include <fg/domain.hh>
#include <fg/clonable.hh>
#include <fg/default-operator.hh>
#include <fg/debug.hh>
#include <fg/logarithmetics.hh>

// data type for saving natural statistics
////////////////////////////////////////////////////////////////////////////////

class statistics_t : public std::vector<double> {
public:
        typedef std::vector<double> base_t;

        statistics_t() :
                base_t(),
                n     (0)
                { }
        statistics_t(size_t k) :
                base_t(k, 0.0),
                n     (0)
                { }
        statistics_t(const std::vector<double>& v) :
                base_t(v),
                n     (1)
                { }
        statistics_t& operator+=(const statistics_t& rhs) {
                assert(base_t::size() == rhs.size());
                std::transform(rhs.begin(), rhs.end(), base_t::begin(), base_t::begin(),
                               std::plus<double>());
                n += rhs.n;
                return *this;
        }
        void clear() {
                std::fill(base_t::begin(), base_t::end(), 0.0);
                n = 0;
        }
        // the number of data points this statistics was computed from
        size_t n;
};

// what every exponential family should provide
////////////////////////////////////////////////////////////////////////////////
class exponential_family_i : public virtual clonable {
public:
        typedef std::vector<double> vector_t;

        virtual ~exponential_family_i() { }

        virtual exponential_family_i* clone() const = 0;

        virtual exponential_family_i& operator=(const exponential_family_i& exponential_family) = 0;

        virtual double base_measure(const vector_t& x) const = 0;
        virtual double log_partition() const = 0;
        virtual bool renormalize() = 0;
        virtual double entropy() const = 0;
        virtual statistics_t statistics(const vector_t& x) const = 0;
        // these are the moments of the sufficient statistics
        virtual vector_t moments() const = 0;
        virtual const vector_t& parameters() const = 0;
        virtual vector_t& parameters() = 0;
        // number of parameters
        virtual size_t k() const = 0;
        virtual exponential_family_i& pow(double d) = 0;
        // for continuous distributions this is the density function,
        // otherwise the call operator returns a probability
        virtual double operator()(const vector_t& x) const = 0;
        virtual exponential_family_i& operator*=(const exponential_family_i& e) = 0;
        virtual bool operator==(const exponential_family_i& rhs) const = 0;
        virtual bool operator!=(const exponential_family_i& rhs) const = 0;
        // the distribution is false if it is not properly initialized
        virtual operator bool() const = 0;
};
inline exponential_family_i* new_clone(const exponential_family_i& a)
{
    return a.clone();
}

// implement what is common to most exponential families
////////////////////////////////////////////////////////////////////////////////
class exponential_family_t : public exponential_family_i {
public:
        // constructors
        // n        : number of times the distribution has been multiplied
        // k        : number of parameters
        exponential_family_t(size_t k, size_t n = 1,
                             const domain_i& domain = real_domain_t(1))
                : _parameters   (k, 0.0),
                  _log_partition(0.0),
                  _domain       (domain.clone()),
                  _n            (n) {
        }
        exponential_family_t(const exponential_family_t& e)
                : _parameters   (e._parameters),
                  _log_partition(e._log_partition),
                  _domain       (e._domain->clone()),
                  _n            (e._n)
                { }
        virtual ~exponential_family_t() {
                delete(_domain);
        }

        // clone object
        virtual exponential_family_t* clone() const = 0;

        friend void swap(exponential_family_t& left,
                         exponential_family_t& right) {
                using std::swap;
                swap(left._parameters,    right._parameters);
                swap(left._log_partition, right._log_partition);
                swap(left._domain,        right._domain);
                swap(left._n,             right._n);
        }

        // operators
        virtual bool operator==(const exponential_family_i& rhs) const {
                const exponential_family_t& tmp = static_cast<const exponential_family_t&>(rhs);
                for (size_t i = 0; i < k(); i++) {
                        if (std::abs(parameters()[i] - tmp.parameters()[i]) > 1.0e-10) {
                                return false;
                        }
                }
                return true;
        }
        virtual bool operator!=(const exponential_family_i& rhs) const {
                return !operator==(rhs);
        }

        // methods
        virtual void clear() {
                std::fill(_parameters.begin(), _parameters.end(), 0.0);
                _log_partition = 0.0;
                _n = 0;
        }
        virtual double operator()(const vector_t& x) const {
                // is x in the domain of this function?
                if (!domain().element(x)) {
                        return 0.0;
                }
                statistics_t T = statistics(x);
                // check dimensionality
                assert(T.size() == k());
                // compute density or probability
                double tmp = 0.0;
                // compute dot product
                for (size_t i = 0; i < k(); i++) {
                        debug(boost::format("-> T[%d]: %f") % i % T[i] << std::endl);
                        tmp += parameters()[i]*T[i];
                }
                return base_measure(x)*std::exp(tmp - log_partition());
        }
        virtual const vector_t& parameters() const {
                return _parameters;
        }
        virtual vector_t& parameters() {
                return _parameters;
        }
        virtual double log_partition() const {
                return _log_partition;
        }
        virtual exponential_family_i& pow(double d) {
                for (size_t i = 0; i < k(); i++) {
                        _parameters[i] *= d;
                }
                // add normalization constants
                _log_partition *= d;
                // recompute moments
                return *this;
        }
        virtual exponential_family_t& operator*=(const exponential_family_i& _e) {
                const exponential_family_t& e =
                        static_cast<const exponential_family_t&>(_e);
                // add parameters
                for (size_t i = 0; i < k(); i++) {
                        _parameters[i] += e.parameters()[i];
                }
                // add normalization constants
                _log_partition += e._log_partition;
                // increment number of multiplications
                _n += e._n;
                // recompute moments
                return *this;
        }
        virtual bool renormalize() {
                // can't renormalize if this is a null object
                if (_n == 0.0) {
                        return false;
                }
                _n = 1.0;
                return true;
        }
        virtual size_t k() const {
                return _parameters.size();
        }
        virtual operator bool() const {
                return n() != 0;
        }
protected:
        virtual const domain_i& domain() const {
                return *_domain;
        }
        virtual const double& n() const {
                return _n;
        }
        virtual double& n() {
                return _n;
        }
        // natural parameters
        vector_t _parameters;
        // normalization constant
        double _log_partition;
        // domain of the density (compact support)
        domain_i* _domain;
        // keep track of the number of multiplications
        double _n;
};

// the normal distribution
////////////////////////////////////////////////////////////////////////////////
class normal_distribution_t : public exponential_family_t {
public:
        typedef exponential_family_t base_t;

        // void object
        normal_distribution_t() :
                base_t(2, 0, real_domain_t(1)) {
        }
        normal_distribution_t(double mean, double precision) :
                base_t(2, 1, real_domain_t(1)) {
                assert(precision > 0.0);
                parameters()[0] = mean*precision;
                parameters()[1] = -0.5*precision;
                renormalize();
        }
        normal_distribution_t(const normal_distribution_t& normal_distribution)
                : base_t(normal_distribution)
                { }

        virtual normal_distribution_t* clone() const {
                return new normal_distribution_t(*this);
        }

        friend void swap(normal_distribution_t& left,
                         normal_distribution_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
        }
        virtual_assignment_operator(normal_distribution_t)
        derived_assignment_operator(normal_distribution_t, exponential_family_i)

        virtual double base_measure(const vector_t& x) const {
                const double m = static_cast<double>(n());
                return std::pow(2.0*M_PI, -m/2.0);
        }
        virtual bool renormalize() {
                if (!base_t::renormalize()) {
                        return false;
                }
                const double mu  = -0.5*parameters()[0]/parameters()[1];
                const double tau = -2.0*parameters()[1];
                debug("-> normal parameters:" << std::endl);
                debug("-> mean     : " << mu  << std::endl);
                debug("-> precision: " << tau << std::endl);
                _log_partition = 1.0/2.0*(tau*mu*mu - std::log(tau));
                return true;
        }
        virtual vector_t moments() const {
                vector_t m(2, 0.0);
                const double mean      = -0.5*parameters()[0]/parameters()[1];
                const double precision = -2.0*parameters()[1];
                m[0] = mean;
                m[1] = mean*mean + 1.0/precision;
                return m;
        }
        virtual double entropy() const {
                const double tau = -2.0*parameters()[1];
                return 1.0/2.0*(1.0 + std::log(2.0*M_PI*1.0/tau));
        }
        virtual statistics_t statistics(double x) const {
                statistics_t T(2);
                T[0] += x;
                T[1] += x*x;
                T.n   = 1;
                return T;
        }
        virtual statistics_t statistics(const vector_t& x) const {
                return statistics(x[0]);
        }
};

// the bivariate normal distribution
////////////////////////////////////////////////////////////////////////////////
class binormal_distribution_t : public exponential_family_t {
public:
        typedef exponential_family_t base_t;

        // void object
        binormal_distribution_t() :
                base_t(6, 0, real_domain_t(2)) {
        }
        binormal_distribution_t(double mux, double muy,
                                double sx, double sy, double rho) :
                base_t(6, 1, real_domain_t(2)) {
                assert(sx > 0.0);
                assert(sy > 0.0);
                assert(-1.0 < rho && rho < 1.0);
                parameters()[0] = 1.0/(1.0 - rho*rho)*(mux*sy - muy*sx*rho)/(sx*sx*sy);
                parameters()[1] = 1.0/(1.0 - rho*rho)*(muy*sx - mux*sy*rho)/(sx*sy*sy);
                parameters()[2] = 1.0/(1.0 - rho*rho)*(-1.0/(2.0*sx*sx));
                parameters()[3] = 1.0/(1.0 - rho*rho)*(rho/(2.0*sx*sy));
                parameters()[4] = 1.0/(1.0 - rho*rho)*(rho/(2.0*sx*sy));
                parameters()[5] = 1.0/(1.0 - rho*rho)*(-1.0/(2.0*sy*sy));
                renormalize();
        }
        binormal_distribution_t(const binormal_distribution_t& binormal_distribution)
                : base_t(binormal_distribution)
                { }

        virtual binormal_distribution_t* clone() const {
                return new binormal_distribution_t(*this);
        }

        friend void swap(binormal_distribution_t& left,
                         binormal_distribution_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
        }
        virtual_assignment_operator(binormal_distribution_t)
        derived_assignment_operator(binormal_distribution_t, exponential_family_i)

        virtual double base_measure(const vector_t& x) const {
                const double m = static_cast<double>(n());
                return std::pow(2.0*M_PI, -m);
        }
        virtual bool renormalize() {
                if (!base_t::renormalize()) {
                        return false;
                }
                const double e1 = parameters()[0];
                const double e2 = parameters()[1];
                const double e3 = parameters()[2];
                const double e4 = parameters()[3];
                const double e5 = parameters()[4];
                const double e6 = parameters()[5];
                _log_partition  = 0.0;
                _log_partition +=  1.0/4.0*(e2*e2*e3 - e1*e2*(e4 + e5) + e1*e1*e6)/(e4*e5 - e3*e6);
                _log_partition += -1.0/2.0*std::log(4.0*e3*e6 - 4.0*e4*e5);
                return true;
        }
        virtual vector_t moments() const {
                vector_t m(6, 0.0);
                const double e1 = parameters()[0];
                const double e2 = parameters()[1];
                const double e3 = parameters()[2];
                const double e4 = parameters()[3];
                const double e5 = parameters()[4];
                const double e6 = parameters()[5];
                const double rho = std::sqrt((e4*e5)/(e3*e6));
                const double sx  = std::sqrt(-1.0/(2.0*(1.0-rho*rho)*e3));
                const double sy  = std::sqrt(-1.0/(2.0*(1.0-rho*rho)*e6));
                const double mux = sx*(    e1*sx + rho*e2*sy);
                const double muy = sy*(rho*e1*sx +     e2*sy);
                m[0] = mux;
                m[1] = muy;
                m[2] = mux*mux + sx*sx;
                m[3] = mux*muy + rho*sx*sy;
                m[4] = mux*muy + rho*sx*sy;
                m[5] = muy*muy + sy*sy;
                return m;
        }
        virtual double entropy() const {
                const double e3 = parameters()[2];
                const double e4 = parameters()[3];
                const double e5 = parameters()[4];
                const double e6 = parameters()[5];
                return 1.0 + std::log(2.0*M_PI) - 1.0/2.0*std::log(4.0*e3*e6 - 4.0*e4*e5);
        }
        virtual statistics_t statistics(const vector_t& x) const {
                const double x1 = x[0];
                const double x2 = x[1];
                statistics_t T(6);
                T[0] = x1;
                T[1] = x2;
                T[2] = x1*x1;
                T[3] = x1*x2;
                T[4] = x2*x1;
                T[5] = x2*x2;
                T.n = 1;
                return T;
        }
};

// the gamma distribution
////////////////////////////////////////////////////////////////////////////////
class gamma_distribution_t : public exponential_family_t {
public:
        typedef exponential_family_t base_t;

        // void object
        gamma_distribution_t() :
                base_t(2, 0, positive_domain_t(1)) {
                parameters()[0] = 0.0;
                parameters()[1] = 0.0;
        }
        gamma_distribution_t(double shape, double rate) :
                base_t(2, 1, positive_domain_t(1)) {
                assert(shape > 0.0);
                assert(rate  > 0.0);
                parameters()[0] =  shape-1.0;
                parameters()[1] = -rate;
                renormalize();
        }
        gamma_distribution_t(const gamma_distribution_t& gamma_distribution)
                : base_t(gamma_distribution)
                { }

        virtual gamma_distribution_t* clone() const {
                return new gamma_distribution_t(*this);
        }

        friend void swap(gamma_distribution_t& left,
                         gamma_distribution_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
        }
        virtual_assignment_operator(gamma_distribution_t)
        derived_assignment_operator(gamma_distribution_t, exponential_family_i)

        virtual double base_measure(const vector_t& x) const {
                return 1.0;
        }
        virtual bool renormalize() {
                if (!base_t::renormalize()) {
                        return false;
                }
                const double a1 =  parameters()[0]+1.0;
                const double a2 = -parameters()[1];
                debug("-> gamma parameters:" << std::endl);
                debug("-> shape: " << a1 << std::endl);
                debug("-> rate : " << a2 << std::endl);
                _log_partition =  boost::math::lgamma(a1) - (a1)*std::log(a2);
                return true;
        }
        virtual vector_t moments() const {
                vector_t m(2, 0.0);
                const double a1 =  parameters()[0]+1.0;
                const double a2 = -parameters()[1];
                m[0] = boost::math::digamma(a1) - std::log(a2);
                m[1] = a1/a2;
                return m;
        }
        virtual double entropy() const {
                const double a1 =  parameters()[0]+1.0;
                const double a2 = -parameters()[1];
                return a1 + boost::math::lgamma(a1) - std::log(a2)
                        + (1.0-a1)*boost::math::digamma(a1);
        }
        virtual statistics_t statistics(double x) const {
                statistics_t T(2);
                T[0] += std::log(x);
                T[1] += x;
                T.n   = 1;
                return T;
        }
        virtual statistics_t statistics(const vector_t& x) const {
                return statistics(x[0]);
        }
};

// the dirichlet distribution
////////////////////////////////////////////////////////////////////////////////
class dirichlet_distribution_t : public exponential_family_t {
public:
        typedef exponential_family_t base_t;

        // void object
        dirichlet_distribution_t(size_t k) :
                base_t(k, 0, simplex_t(k)) {
        }
        dirichlet_distribution_t(const vector_t& alpha) :
                base_t(alpha.size(), 1, simplex_t(alpha.size())) {
                for (size_t i = 0; i < alpha.size(); i++) {
                        assert(alpha[i] > 0.0);
                        parameters()[i] = alpha[i]-1.0;
                }
                renormalize();
        }
        dirichlet_distribution_t(const dirichlet_distribution_t& dirichlet_distribution)
                : base_t(dirichlet_distribution)
                { }

        virtual dirichlet_distribution_t* clone() const {
                return new dirichlet_distribution_t(*this);
        }

        friend void swap(dirichlet_distribution_t& left,
                         dirichlet_distribution_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
        }
        virtual_assignment_operator(dirichlet_distribution_t)
        derived_assignment_operator(dirichlet_distribution_t, exponential_family_i)

        virtual double base_measure(const vector_t& x) const {
                return 1.0;
        }
        virtual bool renormalize() {
                if (!base_t::renormalize()) {
                        return false;
                }
                double sum = std::accumulate(parameters().begin(), parameters().end(), 0.0);
                debug("-> dirichlet parameters:" << std::endl);
                _log_partition = 0.0;
                for (size_t i = 0; i < k(); i++) {
                        const double alpha = parameters()[i]+1.0;
                        debug(boost::format("-> alpha[%i]: %d\n") % i % alpha);
                        _log_partition += boost::math::lgamma(alpha);
                }
                _log_partition -= boost::math::lgamma(sum+k());
                return true;
        }
        virtual vector_t moments() const {
                vector_t m(k(), 0.0);
                double sum = std::accumulate(parameters().begin(), parameters().end(), 0.0);
                for (size_t i = 0; i < k(); i++) {
                        m[i] = boost::math::digamma(parameters()[i]+1.0) -
                                boost::math::digamma(sum+k());
                }
                return m;
        }
        virtual double entropy() const {
                double sum = std::accumulate(parameters().begin(), parameters().end(), 0.0);
                double h   = log_partition() + sum*boost::math::digamma(sum+k());
                for (size_t i = 0; i < k(); i++) {
                        const double alpha = parameters()[i]+1.0;
                        h -= (alpha-1.0)*boost::math::digamma(alpha);
                }
                return h;
        }
        virtual statistics_t statistics(const vector_t& x) const {
                assert(x.size() == k());
                statistics_t T(k());
                for (size_t i = 0; i < x.size(); i++) {
                        T[i] = std::log(x[i]);
                }
                T.n = 1;
                return T;
        }
};

// the categorical distribution
////////////////////////////////////////////////////////////////////////////////
class categorical_distribution_t : public exponential_family_t {
public:
        typedef exponential_family_t base_t;

        // void object
        categorical_distribution_t(size_t k) :
                base_t(k, 0, discrete_domain_t(1)) {
        }
        categorical_distribution_t(const vector_t& theta, bool natural = false) :
                base_t(theta.size(), 1.0, discrete_domain_t(1)) {
                for (size_t i = 0; i < theta.size(); i++) {
                        if (natural) {
                                parameters()[i] = theta[i];
                        }
                        else {
                                assert(theta[i] >= 0.0);
                                parameters()[i] = std::log(theta[i]);
                        }
                }
                renormalize();
        }
        categorical_distribution_t(const categorical_distribution_t& categorical_distribution)
                : base_t(categorical_distribution)
                { }

        virtual categorical_distribution_t* clone() const {
                return new categorical_distribution_t(*this);
        }

        friend void swap(categorical_distribution_t& left,
                         categorical_distribution_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
        }
        virtual_assignment_operator(categorical_distribution_t)
        derived_assignment_operator(categorical_distribution_t, exponential_family_i)

        virtual double base_measure(const vector_t& x) const {
                return 1.0;
        }
        virtual bool renormalize() {
                if (!base_t::renormalize()) {
                        return false;
                }
                double sum = -std::numeric_limits<double>::max();
                debug("-> categorical parameters:" << std::endl);
                for (size_t i = 0; i < k(); i++) {
                        sum = logadd(sum, parameters()[i]);
                }
                for (size_t i = 0; i < k(); i++) {
                        parameters()[i] -= sum;
                        debug(boost::format("-> theta[%i]: %d\n") % i % std::exp(parameters()[i]));
                }
                _log_partition = 0.0;
                return true;
        }
        virtual vector_t moments() const {
                vector_t m(k(), 0.0);
                for (size_t i = 0; i < k(); i++) {
                        m[i] = std::exp(parameters()[i]);
                }
                return m;
        }
        virtual double entropy() const {
                double h = 0.0;
                for (size_t i = 0; i < k(); i++) {
                        h -= std::exp(parameters()[i])*parameters()[i];
                }
                return h;
        }
        virtual statistics_t statistics(double x) const {
                statistics_t T(k());
                assert(x < k());
                T[x] += 1.0;
                T.n = 1;
                return T;
        }
        virtual statistics_t statistics(const vector_t& x) const {
                return statistics(x[0]);
        }
};

#endif /* __TFBAYES_FG_DISTRIBUTION_HH__ */
