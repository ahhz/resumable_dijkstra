//
//=======================================================================
// Copyright 2014
// Author: Alex Hagen-Zanker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// The logging_visitor conforms to the dijkstra visitor concepts and joins 
// writes to an outstream whatever it is doing.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_LOGGING_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_LOGGING_VISITOR_HPP
#include <boost/ref.hpp>
namespace blink {
template<typename OutStream>
struct logging_visitor
{
  logging_visitor(OutStream& os) : m_os(boost::ref(os))
  {}

  boost::reference_wrapper<OutStream> m_os;
  
  template<typename U, typename G>
  void initialize_vertex(const U& u, const G& g)
  {
    m_os.get() << "initialize_vertex " << u << std::endl;
  }

  template<typename U, typename G>
  void examine_vertex(const U& u, const G& g)
  {
    m_os.get() << "examine_vertex    " << u << std::endl;
  }

  template<typename E, typename G>
  void examine_edge(const E& e, const G& g) 
  {
    m_os.get() << "examine_edge      " << source(e, g) << " - " << target(e, g) << std::endl;
  }

  template<typename U, typename G>
  void discover_vertex(const U& u, const G& g)  
  {
    m_os.get() << "discover_vertex   " << u << std::endl;
  }

  template<typename E, typename G>
  void edge_relaxed(const E& e, const G& g) 
  { 
    m_os.get() << "edge_relaxed      " << source(e, g) << " - " << target(e, g) << std::endl;
  }

  template<typename E, typename G>
  void edge_not_relaxed(const E& e, const G& g) 
  { 
    m_os.get() << "edge_not_relaxed  " << source(e, g) << " - " << target(e, g) << std::endl;
  }

  template<typename U, typename G>
  void finish_vertex(const U& u, const G& g) 
  {
    m_os.get() << "finish_vertex     " << u << std::endl;
  }
};
}
#endif