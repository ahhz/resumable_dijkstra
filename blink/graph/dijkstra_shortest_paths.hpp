//
//=======================================================================
// Copyright 2012
// Author: Alex Hagen-Zanker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// The dijkstra shortest path algorithm, without initialization
//
//=======================================================================
//
#ifndef BLINK_DIJKSTRA_SHORTEST_PATH_HPP
#define BLINK_DIJKSTRA_SHORTEST_PATH_HPP

#include <blink/graph/breadth_first_search.hpp>
#include <blink/graph/relax.hpp>

#include <boost/property_map/property_map.hpp>


namespace blink {

namespace detail {
      
// mostly a copy fROm BGL, but uses relax_target and relax_target_confident: https://svn.boost.org/trac/boost/attachment/ticket/7387/minor_improvements.patch
template <class UniformCostVisitor, class UpdatableQueue,
  class WeightMap, class PredecessorMap, class DistanceMap,
  class BinaryFunction, class BinaryPredicate>
struct dijkstra_bfs_visitor
{
  typedef typename boost::property_traits<DistanceMap>::value_type D;

  dijkstra_bfs_visitor(UniformCostVisitor vis, UpdatableQueue& Q,
                        WeightMap w, PredecessorMap p, DistanceMap d,
                        BinaryFunction combine, BinaryPredicate compare,
                        D zero)
    : m_vis(vis), m_Q(Q), m_weight(w), m_predecessor(p), m_distance(d),
      m_combine(combine), m_compare(compare), m_zero(zero)  
  { }

  template <class Edge, class Graph>
  void tree_edge(Edge e, Graph& g) {
    relax_target_confident(e, g, m_weight, m_predecessor, m_distance,
                            m_combine);
    m_vis.edge_relaxed(e, g);
  }     
      
  template <class Edge, class Graph>
  void gray_target(Edge e, Graph& g) {
    bool decreased = relax_target(e, g, m_weight, m_predecessor, m_distance,
                          m_combine, m_compare);
    if (decreased) {
      m_Q.update(target(e, g));
      m_vis.edge_relaxed(e, g);
    } else
      m_vis.edge_not_relaxed(e, g);
  }

  template <class Vertex, class Graph>
  void initialize_vertex(Vertex u, Graph& g)
  { 
	  m_vis.initialize_vertex(u, g); 
  }
      
  template <class Edge, class Graph>
  void non_tree_edge(Edge, Graph&) 
  { }
      
  template <class Vertex, class Graph>
  void discover_vertex(Vertex u, Graph& g) 
  { 
	  m_vis.discover_vertex(u, g); 
  }
      
  template <class Vertex, class Graph>
  void examine_vertex(Vertex u, Graph& g) 
  { 
	  m_vis.examine_vertex(u, g); 
  }
      
  template <class Edge, class Graph>
  void examine_edge(Edge e, Graph& g) 
  {
    if (m_compare(get(m_weight, e), m_zero)) {
        boost::throw_exception(boost::negative_edge());
    }
    m_vis.examine_edge(e, g);
  }
      
  template <class Edge, class Graph>
  void black_target(Edge, Graph&) 
  { }

  template <class Vertex, class Graph>
  void finish_vertex(Vertex u, Graph& g) 
  { 
	  m_vis.finish_vertex(u, g); 
  }

  UniformCostVisitor m_vis;
  UpdatableQueue& m_Q;
  WeightMap m_weight;
  PredecessorMap m_predecessor;
  DistanceMap m_distance;
  BinaryFunction m_combine;
  BinaryPredicate m_compare;
  D m_zero;
};

} // namespace detail

// Call breadth first search
template <class Graph, class DijkstraVisitor, class PredecessorMap, 
  class DistanceMap, class WeightMap, class IndexMap, class Compare, 
  class Combine, class DistZero, class ColorMap, class MutableQueue, 
  class Interruptor>
inline void
  dijkstra_shortest_paths_no_init_at_all( const Graph& g, 
    PredecessorMap predecessor, DistanceMap distance, WeightMap weight,
    IndexMap index_map, Compare compare, Combine combine, DistZero zero,
    DijkstraVisitor vis, ColorMap color, MutableQueue& queue, 
    Interruptor interruptor)
{
  detail::dijkstra_bfs_visitor<DijkstraVisitor, MutableQueue, WeightMap,
    PredecessorMap, DistanceMap, Combine, Compare>
    bfs_vis(vis, queue, weight, predecessor, distance, combine, compare, zero);

  breadth_first_visit_no_init(g, queue, bfs_vis, color, interruptor);
}

} // namespace blink

#endif
