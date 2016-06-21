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
// The nearest source visitor implements the dijkstra visitor concepts and 
// records for each node which is the nearest source. Only useful for a 
// search with multiple sources.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_NEAREST_SOURCE_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_NEAREST_SOURCE_VISITOR_HPP

#include <blink/graph/property_maps/vertex_property_map_helper.hpp>
#include <boost/tuple/tuple.hpp> //tie
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace blink {
  
template<typename NearestsourceMap>  
struct nearest_source_visitor : public boost::default_dijkstra_visitor
{
  nearest_source_visitor(const NearestsourceMap& nearest = NearestsourceMap() ) 
    : m_nearest_source_map(nearest)
  {}
  template<typename Graph>
  void init_map(const Graph& g) 
  {
    typename boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      put(m_nearest_source_map, *ui, *ui);
    }
  }

  template<typename E, typename G>
  void edge_relaxed(const E& e, const G& g) 
  {
    put(m_nearest_source_map, target(e,g), get(m_nearest_source_map, source(e,g))); 
  }

  NearestsourceMap m_nearest_source_map;
};

template<typename Graph, typename NearestsourceMap>
void init_nearest_source_map(const Graph& g, NearestsourceMap nearest)
{
  typename graph_traits<Graph>::vertex_iterator ui, ui_end;
  for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
    put(nearest, *ui, *ui);
  }
}

template<typename DijkstraState>
struct nearest_source_helper
{
  typedef typename DijkstraState::template param<boost::vertex_index_t>::type index_type;
  typedef typename DijkstraState::graph_type graph_type;

  typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
  typedef typename vertex_property_map_helper<vertex_descriptor,graph_type, index_type> helper; 
  typedef typename helper::type map_type;
  typedef typename nearest_source_visitor<map_type> visitor_type;
 
  static map_type make_map(DijkstraState& state)
  {
    return helper::make(state.get_graph(), state.get<boost::vertex_index_t>());
  }

    static visitor_type make_visitor(DijkstraState& state)
  {
    map_type map = make_map(state);
    visitor_type visitor(map);
    visitor.init_map(state.get_graph());
      
    return visitor;
  }
};

template<typename Graph, typename Params>
struct nearest_source_helper_indirect : nearest_source_helper<typename dijkstra_state_helper<Graph, Params>::type>
{};

template<typename DijkstraState>
typename nearest_source_helper<DijkstraState>::visitor_type 
  make_nearest_source_visitor(DijkstraState& state)
{
  return nearest_source_helper<DijkstraState>::make_visitor(state);
}


};// namespace blink;

#endif //BLINK_GRAPH_DIJKSTRA_VISITOR_NEAREST_SOURCE_VISITOR_HPP