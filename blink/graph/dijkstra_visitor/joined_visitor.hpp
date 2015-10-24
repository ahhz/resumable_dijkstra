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
// The joined_visitor conforms to the dijkstra visitor concepts and joins 
// two visitors into one. Executing the call-backs of both.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_JOINED_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_JOINED_VISITOR_HPP
#include <tuple>
namespace blink {

template<typename Vis1, typename Vis2>
struct joined_visitor
{
  // Constructor copies both visitors
  joined_visitor(const Vis1& vis1, const Vis2& vis2)
    : m_vis1(vis1), m_vis2(vis2) 
  {  
  }

  template<typename U, typename G>
  void initialize_vertex(const U& u, const G& g)
  {
    m_vis1.initialize_vertex(u, g);
    m_vis2.initialize_vertex(u, g);
  }

  template<typename U, typename G>
  void examine_vertex(const U& u, const G& g)
  {
    m_vis1.examine_vertex(u, g);
    m_vis2.examine_vertex(u, g);
  }

  template<typename E, typename G>
  void examine_edge(const E& e, const G& g) 
  {
    m_vis1.examine_edge(e, g);
    m_vis2.examine_edge(e, g);
  }

  template<typename U, typename G>
  void discover_vertex(const U& u, const G& g)  
  {
    m_vis1.discover_vertex(u, g);
    m_vis2.discover_vertex(u, g);
  }

  template<typename E, typename G>
  void edge_relaxed(const E& e, const G& g) 
  { 
    m_vis1.edge_relaxed(e, g);
    m_vis2.edge_relaxed(e, g);
  }

  template<typename E, typename G>
  void edge_not_relaxed(const E& e, const G& g) 
  { 
    m_vis1.edge_not_relaxed(e, g);
    m_vis2.edge_not_relaxed(e, g);
  }

  template<typename U, typename G>
  void finish_vertex(const U& u, const G& g) 
  {
    m_vis1.finish_vertex(u, g);
    m_vis2.finish_vertex(u, g);
  }

  Vis1 m_vis1;
  Vis2 m_vis2;
};

template<typename Vis1, typename Vis2>
joined_visitor<Vis1, Vis2> make_joined_visitor(Vis1 a, Vis2 b)
{
  return joined_visitor<Vis1, Vis2>(a,b);
}


} // namespace

#endif //BLINK_GRAPH_DIJKSTRA_VISITOR_JOINED_VISITOR_HPP
