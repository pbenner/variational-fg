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

#ifndef __TFBAYES_FG_VARIATIONAL_HH__
#define __TFBAYES_FG_VARIATIONAL_HH__

#ifdef HAVE_CONFIG_H
#include <fg/config.h>
#endif /* HAVE_CONFIG_H */

#include <boost/ptr_container/ptr_vector.hpp>

#include <fg/node-set.hh>
#include <fg/node-types.hh>
#include <fg/default-operator.hh>

// variable node specializations
////////////////////////////////////////////////////////////////////////////////
class normal_vnode_t : public variable_node_t<normal_distribution_t> {
public:
        typedef variable_node_t<normal_distribution_t> base_t;

        normal_vnode_t(std::string name)
                : base_t(normal_distribution_t(), name)
                { }

        virtual normal_vnode_t* clone() const {
                return new normal_vnode_t(*this);
        }

        virtual double init(boost::random::mt19937& generator);
};

class gamma_vnode_t : public variable_node_t<gamma_distribution_t> {
public:
        typedef variable_node_t<gamma_distribution_t> base_t;

        gamma_vnode_t(std::string name)
                : base_t(gamma_distribution_t(), name)
                { }

        virtual gamma_vnode_t* clone() const {
                return new gamma_vnode_t(*this);
        }

        virtual double init(boost::random::mt19937& generator);
};

class dirichlet_vnode_t : public variable_node_t<dirichlet_distribution_t> {
public:
        typedef variable_node_t<dirichlet_distribution_t> base_t;

        dirichlet_vnode_t(std::string name, size_t k)
                : base_t(dirichlet_distribution_t(k), name)
                { }

        virtual dirichlet_vnode_t* clone() const {
                return new dirichlet_vnode_t(*this);
        }

        virtual double init(boost::random::mt19937& generator);
};

class categorical_vnode_t : public variable_node_t<categorical_distribution_t> {
public:
        typedef variable_node_t<categorical_distribution_t> base_t;

        categorical_vnode_t(std::string name, size_t k)
                : base_t(categorical_distribution_t(k), name)
                { }

        virtual categorical_vnode_t* clone() const {
                return new categorical_vnode_t(*this);
        }

        virtual double init(boost::random::mt19937& generator);
};

// factor node specializations
////////////////////////////////////////////////////////////////////////////////
class normal_fnode_t : public factor_node_t {
public:
        typedef factor_node_t base_t;

        normal_fnode_t(const std::string& name,
                       double mean = 0.0, double precision = 1.0);
        normal_fnode_t(const normal_fnode_t& normal_fnode);

        virtual normal_fnode_t* clone() const;

        friend void swap(normal_fnode_t& left,
                         normal_fnode_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
                swap(left.dmean,         right.dmean);
                swap(left.dprecision,    right.dprecision);
                swap(left.distribution1, right.distribution1);
                swap(left.distribution2, right.distribution2);
                swap(left.distribution3, right.distribution3);
        }
        virtual_assignment_operator(normal_fnode_t);
        derived_assignment_operator(normal_fnode_t, factor_node_i);

        virtual double operator()();

protected:
        virtual bool is_conjugate(size_t i, variable_node_i& variable_node) const;
        virtual p_message_t& message(size_t i);

        // message preparation
        p_message_t& message1();
        p_message_t& message2();
        p_message_t& message3();

        // parameters
        q_message_t dmean;
        q_message_t dprecision;

        // messages
        normal_distribution_t distribution1;
        normal_distribution_t distribution2;
         gamma_distribution_t distribution3;
};

class gamma_fnode_t : public factor_node_t {
public:
        typedef factor_node_t base_t;

        gamma_fnode_t(const std::string& name,
                      double shape = 1.0, double rate = 1.0);
        gamma_fnode_t(const gamma_fnode_t& gamma_fnode);

        virtual gamma_fnode_t* clone() const;

        friend void swap(gamma_fnode_t& left,
                         gamma_fnode_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
                swap(left.dshape,        right.dshape);
                swap(left.drate,         right.drate);
                swap(left.distribution1, right.distribution1);
                swap(left.distribution3, right.distribution3);
        }
        virtual_assignment_operator(gamma_fnode_t);
        derived_assignment_operator(gamma_fnode_t, factor_node_i);

        virtual double operator()();

protected:
        virtual bool is_conjugate(size_t i, variable_node_i& variable_node) const;
        virtual p_message_t& message(size_t i);

        // message preparation
        p_message_t& message1();
        p_message_t& message3();

        // parameters
        q_message_t dshape;
        q_message_t drate;

        // messages
        gamma_distribution_t distribution1;
        gamma_distribution_t distribution3;
};

class dirichlet_fnode_t : public factor_node_t {
public:
        typedef exponential_family_i::vector_t vector_t;
        typedef factor_node_t base_t;

        dirichlet_fnode_t(const std::string& name,
                          const vector_t& alpha);
        dirichlet_fnode_t(const dirichlet_fnode_t& dirichlet_fnode);

        virtual dirichlet_fnode_t* clone() const;

        friend void swap(dirichlet_fnode_t& left,
                         dirichlet_fnode_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
                swap(left.dalpha,        right.dalpha);
                swap(left.distribution1, right.distribution1);
        }
        virtual_assignment_operator(dirichlet_fnode_t);
        derived_assignment_operator(dirichlet_fnode_t, factor_node_i);

        virtual double operator()();

protected:
        virtual bool is_conjugate(size_t i, variable_node_i& variable_node) const;
        virtual p_message_t& message(size_t i);

        // parameters
        q_message_t dalpha;

        // message preparation
        p_message_t& message1();

        // messages
        dirichlet_distribution_t distribution1;
};

class categorical_fnode_t : public factor_node_t {
public:
        typedef exponential_family_i::vector_t vector_t;
        typedef factor_node_t base_t;

        categorical_fnode_t(const std::string& name, const vector_t& theta);
        categorical_fnode_t(const std::string& name, size_t k);
        categorical_fnode_t(const categorical_fnode_t& categorical_fnode);

        virtual categorical_fnode_t* clone() const;

        friend void swap(categorical_fnode_t& left,
                         categorical_fnode_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
                swap(left.dtheta,        right.dtheta);
                swap(left.distribution1, right.distribution1);
                swap(left.distribution2, right.distribution2);
        }
        virtual_assignment_operator(categorical_fnode_t);
        derived_assignment_operator(categorical_fnode_t, factor_node_i);

        virtual double operator()();

protected:
        virtual bool is_conjugate(size_t i, variable_node_i& variable_node) const;
        virtual p_message_t& message(size_t i);

        // parameters
        q_message_t dtheta;

        // message preparation
        p_message_t& message1();
        p_message_t& message2();

        // messages
        categorical_distribution_t distribution1;
          dirichlet_distribution_t distribution2;
};

class mixture_fnode_t : public factor_node_t {
public:
        typedef exponential_family_i::vector_t vector_t;
        typedef boost::ptr_vector<factor_node_i> factor_nodes_t;
        typedef factor_node_t base_t;

        mixture_fnode_t(const std::string& name);
        mixture_fnode_t(const mixture_fnode_t& mixture_fnode);

        virtual mixture_fnode_t* clone() const;

        friend void swap(mixture_fnode_t& left,
                         mixture_fnode_t& right) {
                using std::swap;
                swap(static_cast<base_t&>(left),
                     static_cast<base_t&>(right));
                swap(left._factor_nodes, right._factor_nodes);
                swap(left._neighbors,    right._neighbors);
                swap(left.distribution1, right.distribution1);
        }
        virtual_assignment_operator(mixture_fnode_t);
        derived_assignment_operator(mixture_fnode_t, factor_node_i);

        mixture_fnode_t& operator+=(const factor_node_i& factor_node);
        mixture_fnode_t& operator+=(      factor_node_i* factor_node);

        virtual const neighbors_t& neighbors() const;

        virtual bool link(const std::string& tag1, const std::string& tag2, variable_node_i& variable_node);
        virtual bool link(const std::string& tag, variable_node_i& variable_node, p_map_t f);
        virtual double operator()();

protected:
        virtual bool is_conjugate(size_t i, variable_node_i& variable_node) const;
        // message from the mixture node to a connected categorical distribution
        virtual p_message_t& message(size_t i);
        // relay for messages sent to factors inside the mixture
        virtual p_message_t& message(size_t k, p_message_t& p_message);

        // list of factor nodes
        factor_nodes_t _factor_nodes;

        // keep track of neighboring nodes for cloning whole networks
        neighbors_t _neighbors;

        // messages
        categorical_distribution_t distribution1;
};

#endif /* __TFBAYES_FG_VARIATIONAL_HH__ */
