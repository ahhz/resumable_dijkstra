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
// A wrapper to allow mutable boost.heap heaps to be used for the
// max_priority_queue in the dijkstra algorithms.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_HEAP_WRAPPER_HPP
#define BLINK_GRAPH_DIJKSTRA_HEAP_WRAPPER_HPP

#include <blink/graph/dijkstra_parameter_helper.hpp> //property_map_helper

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace blink {
template<typename MutableQueue, typename Graph, typename VertexIndexMap>
struct dijkstra_heap_wrapper
{
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef typename boost::graph_traits<Graph>::traversal_category traversal_category;
  typedef typename MutableQueue::handle_type handle_type;
	typedef typename MutableQueue::value_compare compare_type;
  typedef typename vertex_property_map_helper<handle_type,Graph, VertexIndexMap> helper;
  typedef typename helper::type handle_map_type;

  BOOST_STATIC_ASSERT((boost::is_same<vertex_descriptor
    , typename MutableQueue::value_type>::value));
  BOOST_STATIC_ASSERT((boost::is_same<vertex_descriptor
    , typename boost::property_traits<VertexIndexMap>::key_type>::value));

public:
	// Assumes that the compare_type is an indirect cmp
  template<typename VertexDistanceMap>
  dijkstra_heap_wrapper(const Graph& g, VertexIndexMap index, VertexDistanceMap distance) 
    : heap(compare_type(distance))
  {
    handle_map = helper::make(g, index);
	}			

  const vertex_descriptor& top() const
	{
    return heap.top();
  }

  void pop() 
  {
    heap.pop();
  }
	
  void push(const vertex_descriptor& v) 
  {
    put(handle_map, v, heap.push(v) );
  }
	
  void update(const vertex_descriptor& v)
  {
    heap.increase(get(handle_map, v) );
  }

  bool empty() const
  {
    return heap.empty();
  }

  inline void clear()
  {
    heap.clear();
  }
	
  MutableQueue heap;
  handle_map_type handle_map;
};
}// namespace blink

#endif//BLINK_GRAPH_DIJKSTRA_HEAP_WRAPPER_HPP