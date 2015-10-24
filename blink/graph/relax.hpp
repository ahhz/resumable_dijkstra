//
//=======================================================================
// Copyright 2012-2014
// Author: Alex Hagen-Zanker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// Two specialized relax functions that, when appropriate, are more efficient
// than the boost relax function. relax_target (skips relaxing the origin 
// of a vertex). relax_target_confident (assumes that the target distance 
// decreases).
//
// This file can be deprecated once the following patch is applied:
// https://svn.boost.org/trac/boost/ticket/7387
//=======================================================================
//
#ifndef BLINK_GRAPH_RELAX_HPP
#define BLINK_GRAPH_RELAX_HPP

#include <functional>

#include <boost/limits.hpp> // for numeric limits
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

namespace blink {

template <class Graph, class WeightMap, 
    class PredecessorMap, class DistanceMap, 
    class BinaryFunction, class BinaryPredicate>
  bool relax_target(const typename boost::graph_traits<Graph>::edge_descriptor e, 
    const Graph& g, const WeightMap& w, 
    PredecessorMap& p, DistanceMap& d, 
    const BinaryFunction& combine, const BinaryPredicate& compare)
  {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::property_traits<DistanceMap>::value_type D;
    typedef typename boost::property_traits<WeightMap>::value_type W;
    const Vertex u = source(e, g);
    const Vertex v = target(e, g);
    const D& d_u = get(d, u);
    const D d_v = get(d, v);
    const W& w_e = get(w, e);
      
    // The redundant gets in the return statements are to ensure that extra
    // floating-point precision in x87 registers does not lead to relax_target()
    // returning true when the distance did not actually change.
    if ( compare(combine(d_u, w_e), d_v) ) {
      put(d, v, combine(d_u, w_e));
      if(compare(get(d, v), d_v) ) {
        put(p, v, u);
        return true;
      };
    } 
    return false;
  }

  template <class Graph, class WeightMap, class PredecessorMap, class DistanceMap, 
    class BinaryFunction>
  void relax_target_confident(
    const typename boost::graph_traits<Graph>::edge_descriptor e, 
    const Graph& g, const WeightMap& w, PredecessorMap& p, DistanceMap& d, 
    const BinaryFunction& combine)
  {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::property_traits<DistanceMap>::value_type D;
    typedef typename boost::property_traits<WeightMap>::value_type W;
    const Vertex u = source(e, g);
    const Vertex v = target(e, g);
    const D& d_u = get(d, source(e, g));
    const W& w_e = get(w, e);
      
    put(d, v, combine(d_u, w_e));
    put(p, v, u);
  }

} // namespace blink

#endif // BLINK_GRAPH_RELAX_HPP

