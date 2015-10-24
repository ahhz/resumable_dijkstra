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
// dijkstra_state is a class to hold all property_map, parameters, the 
// queue and graph reference, that are used in the dijkstra shortest paths
// algorithm.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_STATE_HPP
#define BLINK_GRAPH_DIJKSTRA_STATE_HPP

#include <blink/graph/dijkstra_parameter_helper.hpp>

#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

namespace blink {

template< typename Graph, typename EdgeWeight, typename VertexIndex
  , typename VertexDistance, typename VertexPredecessor, typename VertexColor
  , typename DistanceCompare, typename DistanceCombine, typename DistanceZero, typename DistanceInf
  , typename MaxPriorityQueue, typename GraphVisitor>
struct dijkstra_state
{
private: 
	typedef typename MaxPriorityQueue queue_stored_type;
	typedef typename processed_type<queue_stored_type>::type queue_type;
	typedef typename processed_type<queue_stored_type>::stored_type_tag queue_stored_type_tag;

public:
    // The queue must be wrapped in either a boost::reference_wrapper or a shared_ptr
  dijkstra_state(const Graph& graph, EdgeWeight weight, VertexIndex index, 
    VertexDistance distance, VertexPredecessor predecessor , VertexColor 
    color, DistanceCompare compare, DistanceCombine combine, DistanceZero zero, 
    DistanceInf inf, MaxPriorityQueue queue, GraphVisitor visitor) 
    : m_graph(graph), m_edge_weight(weight), m_vertex_index(index), 
    m_vertex_distance(distance), m_vertex_predecessor(predecessor), 
    m_vertex_color(color), m_distance_compare(compare), m_distance_combine(combine), 
    m_distance_zero(zero), m_distance_inf(inf), m_max_priority_queue(queue), 
    m_graph_visitor(visitor)
  {}
	
	typedef Graph graph_type;

	template<typename Tag> struct param {};
	template<> struct param<boost::edge_weight_t>        { typedef EdgeWeight        type; };
	template<> struct param<boost::vertex_index_t>       { typedef VertexIndex       type; };
	template<> struct param<boost::vertex_distance_t>    { typedef VertexDistance    type; };
	template<> struct param<boost::vertex_predecessor_t> { typedef VertexPredecessor type; };
	template<> struct param<boost::vertex_color_t>       { typedef VertexColor       type; };
	template<> struct param<boost::distance_compare_t>   { typedef DistanceCompare   type; };
	template<> struct param<boost::distance_combine_t>   { typedef DistanceCombine   type; };
	template<> struct param<boost::distance_inf_t>       { typedef DistanceInf       type; };
	template<> struct param<boost::distance_zero_t>      { typedef DistanceZero      type; };
	template<> struct param<boost::graph_visitor_t>      { typedef GraphVisitor      type; };
	template<> struct param<boost::max_priority_queue_t> { typedef queue_type        type; };

  EdgeWeight&        get(const boost::edge_weight_t&)         { return m_edge_weight; }
	VertexIndex&       get(const boost::vertex_index_t&)        { return m_vertex_index; }
	VertexDistance&    get(const boost::vertex_distance_t&)     { return m_vertex_distance; }
	VertexPredecessor& get(const boost::vertex_predecessor_t&)  { return m_vertex_predecessor; }
	VertexColor&       get(const boost::vertex_color_t&)        { return m_vertex_color; }
	DistanceCompare&   get(const boost::distance_compare_t&)    { return m_distance_compare; }
	DistanceCombine&   get(const boost::distance_combine_t&)    { return m_distance_combine; }
	DistanceInf&       get(const boost::distance_inf_t&)        { return m_distance_inf; }
	DistanceZero&      get(const boost::distance_zero_t&)       { return m_distance_zero; }
	GraphVisitor&      get(const boost::graph_visitor_t&)       { return m_graph_visitor; }

  queue_type&        get(const boost::max_priority_queue_t&)  
	{ 
		return get_queue(queue_stored_type_tag());
	}

	template<typename Tag>
	typename param<Tag>::type& get()
	{
		return get(Tag());
	}

	inline const Graph& get_graph() const 
  { 
    return m_graph; 
  }

private:
	queue_type& get_queue(const stored_ref_tag&)
	{
		return m_max_priority_queue.get();
	}

	queue_type& get_queue(const stored_ptr_tag&)
	{
		return *m_max_priority_queue;
	}

	// The distance in the distance map must be the same as the zero and inf distance types 
	// as well as the weight in the edges.
  typedef typename boost::property_traits<
    typename param<boost::vertex_distance_t>::type>::value_type distance_type;
    
  typedef typename boost::property_traits<
    typename param<boost::edge_weight_t>::type>::value_type distance_type_check;
  
	BOOST_STATIC_ASSERT((boost::is_same<distance_type, 
    typename param<boost::distance_zero_t>::type>::value));
  BOOST_STATIC_ASSERT((boost::is_same<distance_type, 
    typename param<boost::distance_inf_t>::type>::value));
  BOOST_STATIC_ASSERT((boost::is_same<distance_type, distance_type_check>::value));

  const Graph& m_graph; 
  EdgeWeight m_edge_weight;
  VertexPredecessor m_vertex_predecessor;
  VertexDistance m_vertex_distance;
  VertexColor m_vertex_color;
  VertexIndex m_vertex_index;
  DistanceCompare m_distance_compare;
  DistanceCombine m_distance_combine;
  DistanceZero m_distance_zero;
  DistanceInf m_distance_inf;
  MaxPriorityQueue m_max_priority_queue; 
  GraphVisitor m_graph_visitor;
};// dijkstra_state

// extends dijkstra_parameter_helper with the dijkstra_state type
template<typename Graph, typename Params>
struct dijkstra_state_helper : protected dijkstra_parameter_helper<Graph, Params>
{
  typedef typename dijkstra_parameter_helper<Graph, Params> parent;
    
  template<typename Tag>
  struct param : parent::param<Tag>
  {};
    
  typedef dijkstra_state<Graph
    , typename param<boost::edge_weight_t>::stored_type
    , typename param<boost::vertex_index_t>::stored_type
    , typename param<boost::vertex_distance_t>::stored_type
    , typename param<boost::vertex_predecessor_t>::stored_type
    , typename param<boost::vertex_color_t>::stored_type
    , typename param<boost::distance_compare_t>::stored_type
    , typename param<boost::distance_combine_t>::stored_type
    , typename param<boost::distance_zero_t>::stored_type
    , typename param<boost::distance_inf_t>::stored_type
    , typename param<boost::max_priority_queue_t>::stored_type
    , typename param<boost::graph_visitor_t>::stored_type
    > type;
    
  static type make(const Graph& g, const Params& params)
  {
    parent::param<boost::edge_weight_t>::stored_type edge_weight_map 
      = parent::make(boost::edge_weight_t(), params, g);

    parent::param<boost::vertex_index_t>::stored_type vertex_index_map 
      = parent::make(boost::vertex_index_t(), params,  g);

    parent::param<boost::vertex_distance_t>::stored_type vertex_distance_map 
      = parent::make(boost::vertex_distance_t(), params, g, vertex_index_map);

    parent::param<boost::vertex_predecessor_t>::stored_type  vertex_predecessor_map 
      = parent::make(boost::vertex_predecessor_t(), params);

    parent::param<boost::vertex_color_t>::stored_type vertex_color_map 
      = parent::make(boost::vertex_color_t(), params, g, vertex_index_map);
  
    parent::param<boost::distance_compare_t>::stored_type comparison_object 
      = parent::make(boost::distance_compare_t(), params);

    parent::param<boost::distance_inf_t>::stored_type distance_inf_value 
      = parent::make(boost::distance_inf_t(), params);
  
    parent::param<boost::distance_combine_t>::stored_type combine_object 
      = parent::make(boost::distance_combine_t(), params, distance_inf_value);
    
    parent::param<boost::distance_zero_t>::stored_type distance_zero_value 
      = parent::make(boost::distance_zero_t(), params);
  
    parent::param<boost::max_priority_queue_t>::stored_type max_priority_queue_object 
      = parent::make(boost::max_priority_queue_t(), params, g, 
      vertex_distance_map, comparison_object, vertex_index_map);

    parent::param<boost::graph_visitor_t>::stored_type visitor 
      = parent::make(boost::graph_visitor_t(), params);

    return type(g, edge_weight_map, vertex_index_map, 
      vertex_distance_map, vertex_predecessor_map, vertex_color_map,
      comparison_object, combine_object, distance_zero_value, 
      distance_inf_value, max_priority_queue_object, visitor);
  }
}; // struct dijkstra_state_helper

// Make a dijkstra_state object
template <typename Graph, typename Params>
typename dijkstra_state_helper<Graph, Params>::type 
  make_dijkstra_state(const Graph& g, const Params& params)
{
  return dijkstra_state_helper<Graph, Params>::make(g, params); 
}

// Make a dijkstra_state object, using only default parameters
template <typename Graph>
typename dijkstra_state_helper<Graph, boost::no_named_parameters>::type
  make_dijkstra_state(const Graph& g)
{
  return make_dijkstra_state(g, boost::no_named_parameters() );
}

// by deriving from dijktra_state_mixin, a class has easy access to all dijkstra parameters and typedefs 
template<typename DijkstraState>
struct dijkstra_state_mixin
{
  dijkstra_state_mixin(const boost::shared_ptr<DijkstraState>& state)
    : m_dijkstra_state(state) 
    , m_graph              (m_dijkstra_state->get_graph())
    , m_distance_combine   (m_dijkstra_state->get(boost::distance_combine_t()))
    , m_distance_compare   (m_dijkstra_state->get(boost::distance_compare_t()))
    , m_distance_inf       (m_dijkstra_state->get(boost::distance_inf_t()))
    , m_distance_zero      (m_dijkstra_state->get(boost::distance_zero_t()))
    , m_edge_weight        (m_dijkstra_state->get(boost::edge_weight_t()))
    , m_max_priority_queue (m_dijkstra_state->get(boost::max_priority_queue_t()))
    , m_vertex_color       (m_dijkstra_state->get(boost::vertex_color_t()))
    , m_vertex_distance    (m_dijkstra_state->get(boost::vertex_distance_t()))
    , m_vertex_index       (m_dijkstra_state->get(boost::vertex_index_t()))
    , m_vertex_predecessor (m_dijkstra_state->get(boost::vertex_predecessor_t()))
    , m_graph_visitor      (m_dijkstra_state->get(boost::graph_visitor_t()))
  {}
  
  template<typename Tag>
  struct param
  {
    typedef typename DijkstraState::template param<Tag>::type type;
  };
 
protected:
  typedef typename DijkstraState::graph_type graph_type;
  typedef typename param<boost::distance_combine_t    >::type distance_combine_type;
  typedef typename param<boost::distance_compare_t    >::type distance_compare_type;
  typedef typename param<boost::distance_inf_t        >::type distance_inf_type;
  typedef typename param<boost::distance_zero_t       >::type distance_zero_type;
  typedef typename param<boost::edge_weight_t         >::type edge_weight_type;
  typedef typename param<boost::max_priority_queue_t>  ::type max_priority_queue_type;
  typedef typename param<boost::vertex_color_t        >::type vertex_color_type;
  typedef typename param<boost::vertex_distance_t     >::type vertex_distance_type;
  typedef typename param<boost::vertex_index_t        >::type vertex_index_type;
  typedef typename param<boost::vertex_predecessor_t  >::type vertex_predecessor_type;
  typedef typename param<boost::graph_visitor_t       >::type graph_visitor_type;

  typedef typename boost::graph_traits<graph_type> graph_traits;
  typedef typename graph_traits::vertex_descriptor vertex_descriptor;
  typedef typename graph_traits::vertex_iterator   vertex_iterator;
  typedef typename graph_traits::edge_descriptor   edge_descriptor;
  typedef typename graph_traits::out_edge_iterator out_edge_iterator;

  typedef typename boost::property_traits<vertex_distance_type>::value_type distance_type;
  typedef typename boost::property_traits<vertex_color_type   >::value_type color_type;

  typedef typename boost::color_traits<color_type> color_traits;

public:

 
  const DijkstraState& get_dijkstra_state() const
  {
    return *m_dijkstra_state;
  }

  DijkstraState& get_dijkstra_state()
  {
    return *m_dijkstra_state;
  }

  template<typename Tag>
  typename param<Tag>::type& get(Tag tag = Tag())
  {
    return m_dijkstra_state->get<Tag>();
  }

  template<typename Tag>
  const typename param<Tag>::type& get(Tag tag = Tag()) const
  {
    return m_dijkstra_state->get<Tag>();
  }

  inline const graph_type& get_graph() const
  {
    return graph;
  }

private:
  boost::shared_ptr<DijkstraState> m_dijkstra_state; // MUST BE DECLARED BEFORE ALL REFERENCES

protected:
  const graph_type& m_graph;
  distance_combine_type&   m_distance_combine;
  distance_compare_type&   m_distance_compare;
  distance_inf_type&       m_distance_inf;
  distance_zero_type&      m_distance_zero;
  edge_weight_type&        m_edge_weight;
  graph_visitor_type&      m_graph_visitor;
  max_priority_queue_type& m_max_priority_queue;
  vertex_color_type&       m_vertex_color;
  vertex_distance_type&    m_vertex_distance;
  vertex_index_type&       m_vertex_index;
  vertex_predecessor_type& m_vertex_predecessor;
};
}// namespace blink

#endif //BLINK_GRAPH_DIJKSTRA_STATE_HPP