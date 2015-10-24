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
// A helper to get the type of and make the bgl direct_d_ary_heap for use 
// in the shortest path algorithm.
//
//=======================================================================
//
#ifndef BLINK_GRAPH_DIJKSTRA_QUEUE_HPP
#define BLINK_GRAPH_DIJKSTRA_QUEUE_HPP

#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp> // color_traits
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp> //tie

#include <vector>

namespace blink {

template <typename T>
struct has_clear
{
private:
  typedef char yes_type[1];
  typedef char no_type[2];

  template <typename S, void( S::*)(void)>
  struct dummy {};

  template <typename S>
  static yes_type& check(dummy<S, &S::clear>*);

  template <typename S>
  static no_type& check (...);

public:
  static bool const value = sizeof( check<T>(0) ) == sizeof( yes_type );
  typedef boost::integral_constant<bool, value> type;
}; 

template<typename Queue>
void clear(Queue& queue, boost::false_type)
{
  while(!queue.empty()) {
    queue.pop();
  }
}
 
template<typename Queue>
void clear(Queue& queue, boost::true_type)
{
  queue.clear();
}

template<typename Queue>
void clear(Queue& queue)
{
  typedef typename has_clear<Queue>::type has;
  return clear(queue, has());
}
 
template <typename Graph, typename DistanceMap, typename IndexMap,
  typename Compare>
struct dijkstra_queue_bgl   
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename boost::shared_array_property_map<std::size_t, IndexMap> 
    index_in_heap_map;
  
  typedef boost::d_ary_heap_indirect<vertex_descriptor, 4, index_in_heap_map, 
    DistanceMap, Compare, std::vector<vertex_descriptor> > type;

  // typedef boost::d_ary_heap_indirect<vertex_descriptor, 4, index_in_heap_map, 
  //   DistanceMap, Compare, std::vector<vertex_descriptor>& > ref_type;
  
  static boost::shared_ptr<type> make_smart(const Graph& graph, DistanceMap distance, Compare compare,
    IndexMap indexmap)
  {
    return boost::shared_ptr<type>(new type(distance, index_in_heap_map(num_vertices(graph), indexmap)
      , compare) );
  }
  
  static type make(const Graph& graph, DistanceMap distance, Compare compare,
    IndexMap indexmap)
  {
    return type(distance, index_in_heap_map(num_vertices(graph), indexmap)
      , compare);
  }
};


template<typename Graph, typename ColorMap, typename DijkstraQueue>
void reset_dijkstra_queue_by_colormap(
      ColorMap colormap,  
      const Graph& graph,
    DijkstraQueue& queue)
{
  typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
  typedef typename boost::property_traits<ColorMap>::value_type color_type;
  typedef boost::color_traits<color_type> color_traits;
  
  clear(queue);
  
  vertex_iterator vi, vi_end;
  for (boost::tie(vi, vi_end) = boost::vertices(graph); vi != vi_end; ++vi) {
    if(get(colormap, *vi) == color_traits::gray()) {
      queue.push(*vi); 
    } 
  }
}
}; // namespace blink


#endif // BLINK_GRAPH_DIJKSTRA_QUEUE_HPP