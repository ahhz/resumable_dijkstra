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
// The predecessor edge visitor is intended to assign based on 
// dijkstra_shortest_paths when the graph contains parallel edges.
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_PREDECESSOR_EDGE_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_PREDECESSOR_EDGE_VISITOR_HPP

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/properties.hpp>

namespace blink{

  template<typename PredecessorEdgeMap>
  class predecessor_edge_visitor : public boost::default_dijkstra_visitor
  {
  public:
    typedef typename boost::property_traits<PredecessorEdgeMap>::value_type edge_descriptor;
    typedef typename boost::property_traits<PredecessorEdgeMap>::key_type vertex_descriptor;
    
    predecessor_edge_visitor(PredecessorEdgeMap pem) : m_predecessor_edge_map(pem)
    {}

    template<typename EdgeDescriptor, typename Graph>
    void edge_relaxed(const EdgeDescriptor& e, const Graph& g)
    {
      vertex_descriptor v = boost::target(e);
      put(m_predecessor_edge_map, v, e);
    }

    PredecessorEdgeMap get_map()
    {
      return m_predecessor_edge_map;
    }

  private:
    PredecessorEdgeMap m_predecessor_edge_map
  };
  

  template<typename DijkstraState>
  predecessor_edge_visitor<blink::vertex_property_map_helper<
    boost::graph_traits<DijkstraState::graph_type>::edge_descriptor, 
    DijkstraState::graph_type,
    DijkstraState::param<boost::vertex_index_t>::type>::type >
    make_predecessor_edge_visitor(const DijkstraState& state)
  {
    typedef blink::vertex_property_map_helper<
      boost::graph_traits<DijkstraState::graph_type>::edge_descriptor,
      DijkstraState::graph_type,
      DijkstraState::param<boost::vertex_index_t>::type pem_type;

    typedef predecessor_edge_visitor < pem_type > visitor_type;

    pem_type pem = pem_type::make(state.get_graph()), state.get<boost::vertex_index_t>());
 
    return visitor_type(pem);
  }

  template<
    typename Graph, 
    typename VertexIndexMap = boost::property_map<Graph, boost::vertex_index_t>::type >
  predecessor_edge_visitor<
    blink::vertex_property_map_helper<
      boost::graph_traits<Graph>::edge_descriptor, 
      Graph, 
      VertexIndexMap>::type> >
    make_predecessor_edge_visitor(const Graph& graph, VertexIndexMap vm = boost::get(graph, boost::vertex_index_map_t())
  {
    typedef blink::vertex_property_map_helper< boost::graph_traits<Graph>::edge_descriptor,
      Graph, VertexIndexMap>::type pem_type;

    typedef predecessor_edge_visitor < pem_type > visitor_type;

    pem_type pem = pem_type::make(graph, vm);

    return visitor_type(pem);
  }

} 

#endif //BLINK_GRAPH_DIJKSTRA_VISITOR_TARGET_VISITOR_HPP