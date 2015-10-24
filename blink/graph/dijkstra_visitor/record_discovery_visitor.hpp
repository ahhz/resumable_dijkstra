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
// The record discovery visitor implements the dijkstra visitor concepts 
// and is used to keep a list of all discovered nodes.
//
//=======================================================================
//


#ifndef BLINK_GRAPH_RECORD_DISCOVERY_VISITOR_HPP
#define BLINK_GRAPH_RECORD_DISCOVERY_VISITOR_HPP

#include <boost/graph/dijkstra_shortest_paths.hpp> //default_dijkstra_visitor

namespace blink {
  
template<typename VertexBackInserter>  
struct record_discovery_visitor : public boost::default_dijkstra_visitor
{
  record_discovery_visitor(VertexBackInserter discovery) 
    : m_discovery(discovery)
  {}
  
  template<typename U, typename G>
  void discover_vertex(const U& u, const G& g) 
  { 
    m_discovery = u; 
  }

  VertexBackInserter m_discovery;
};

template<typename VertexBackInserter>  
record_discovery_visitor<VertexBackInserter> 
  make_record_discovery_visitor(VertexBackInserter vbi)
{
  return record_discovery_visitor<VertexBackInserter>(vbi);
}

} // namespace 

#endif // BOOST_GRAPH_RECORD_DISCOVERY_VISITOR_HPP