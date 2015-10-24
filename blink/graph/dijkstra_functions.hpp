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
// Provides convenience function for common dijkstra based functions. 
// "Plain", "Selected Targets", "Multiple Sources", "Selected Distance". 
// The function return a dijkstra_state object and possibly an associated 
// visitor
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_FUNCTIONS_HPP
#define BLINK_GRAPH_DIJKSTRA_FUNCTIONS_HPP

#include <blink/graph/resumable_dijkstra.hpp>
#include <blink/graph/dijkstra_visitor/distance_visitor.hpp>
#include <blink/graph/dijkstra_visitor/target_visitor.hpp>
#include <blink/graph/dijkstra_visitor/nearest_source_visitor.hpp>

#include <utility> //pair

namespace blink {

template<typename Graph, typename VerticesRange, typename Params>
std::pair
  < typename dijkstra_state_helper<Graph, Params>::type
  , typename nearest_source_helper_indirect<Graph, Params>::visitor_type>
dijkstra_shortest_path_nearest_source(const Graph& g, 
  const VerticesRange& sources, const Params& params)
{
  typedef typename dijkstra_state_helper<Graph, Params>::type state_type;
  typedef typename nearest_source_helper<state_type>::visitor_type visitor_type;
    
  state_type state = make_dijkstra_state(g, params);
  visitor_type visitor = make_nearest_source_visitor(state);
   
  resumable_dijkstra<state_type> dijkstra(state);
  dijkstra.init_from_sources(sources, visitor);
  dijkstra.expand(default_interruptor(), visitor);

  return std::make_pair(state, visitor);
}

template<typename Graph, typename VerticesRange>
std::pair
  < typename dijkstra_state_helper<Graph, boost::no_named_parameters>::type
  , typename nearest_source_helper_indirect<Graph, boost::no_named_parameters>::visitor_type>
dijkstra_shortest_path_nearest_source(const Graph& g, const VerticesRange& sources)
{ 
  return dijkstra_shortest_path_nearest_source(g, sources, boost::no_named_parameters());
}

//dijkstra_shortest_path_targets
template<typename Graph, typename VerticesRange, typename Params>
std::pair
  < typename dijkstra_state_helper<Graph, Params>::type
  , typename target_helper_indirect<Graph, Params>::visitor_type>
 
  dijkstra_shortest_path_targets(const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source, 
  const VerticesRange& targets, const Params& params)
{
  typedef typename dijkstra_state_helper<Graph, Params>::type state_type;
  typedef typename target_helper<state_type>::visitor_type visitor_type;
  state_type state = make_dijkstra_state(g, params); 
  visitor_type visitor = make_target_visitor(state);
  visitor.add_targets(targets);
  resumable_dijkstra<state_type> dijkstra(state);
  dijkstra.init_from_source(source, visitor);
  dijkstra.expand(visitor, visitor);
  return std::make_pair(state, visitor);
}

template<typename Graph, typename VerticesRange>
std::pair
  < typename dijkstra_state_helper<Graph, boost::no_named_parameters>::type
  , typename target_helper_indirect<Graph, boost::no_named_parameters>::visitor_type>
 
  dijkstra_shortest_path_targets(const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source, 
  const VerticesRange& targets)
{
  return dijkstra_shortest_path_targets(g, source, targets, boost::no_named_parameters()); 
}


//dijkstra_shortest_path_distance
template<typename Graph, typename DistanceValue, typename Params>
std::pair
  < typename dijkstra_state_helper<Graph, Params>::type
  , typename distance_visitor_helper_indirect<Graph, Params>::type>
dijkstra_shortest_path_distance(
  const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source, 
  DistanceValue target_distance,
  const Params& params)
{
  typedef typename dijkstra_state_helper<Graph, Params>::type state_type;
  typedef typename distance_visitor_helper<state_type>::type visitor_type;
  state_type state = make_dijkstra_state(g, params);
  visitor_type visitor = make_distance_visitor(state, target_distance);
  resumable_dijkstra<state_type> dijkstra(state);
  dijkstra.init_from_source(source, visitor);
  dijkstra.expand(visitor, visitor);
  return std::make_pair(state, visitor);
}
 
template<typename Graph, typename DistanceValue>
std::pair
  < typename dijkstra_state_helper<Graph, boost::no_named_parameters>::type
  , typename distance_visitor_helper_indirect<Graph, boost::no_named_parameters>::type>
dijkstra_shortest_path_distance(
  const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source, 
  DistanceValue target_distance)
{
  return dijkstra_shortest_path_distance(g, source, target_distance, boost::no_named_parameters());
}

//dijkstra_shortest_path_plain
template<typename Graph, typename Params>
typename dijkstra_state_helper<Graph, Params>::type
dijkstra_shortest_path_plain(const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source, const Params& params)
{
  typedef typename dijkstra_state_helper<Graph, Params>::type state_type;
  state_type state = dijkstra_state_helper<Graph, Params>::make(g, params);
  resumable_dijkstra<state_type> dijkstra(state);
  dijkstra.init_from_source(source);
  dijkstra.expand();
  return state;
}

template<typename Graph>
typename dijkstra_state_helper<Graph, boost::no_named_parameters>::type
dijkstra_shortest_path_plain(const Graph& g, 
  typename boost::graph_traits<Graph>::vertex_descriptor source)
{
  return dijkstra_shortest_path_plain(g, source, boost::no_named_parameters());
}
   
} // namespace boost

#endif //BLINK_GRAPH_DIJKSTRA_FUNCTIONS_HPP