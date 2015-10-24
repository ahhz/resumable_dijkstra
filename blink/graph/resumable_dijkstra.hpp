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
// A class that allow interrupting and resuming the dijkstra algorithm. 
//
//=======================================================================
//

#ifndef BLINK_GRAPH_RESUMABLE_DIJKSTRA_HPP
#define BLINK_GRAPH_RESUMABLE_DIJKSTRA_HPP

#include <blink/graph/dijkstra_visitor/joined_visitor.hpp>
#include <blink/graph/dijkstra_state.hpp>
#include <blink/graph/dijkstra_shortest_paths.hpp> //dijkstra_shortest_paths_no_init_at_all
#include <blink/graph/breadth_first_search.hpp> //default_interruptor
#include <blink/graph/relax.hpp> //relax_target

#include <boost/graph/named_function_params.hpp>
#include <boost/range.hpp>
#include <boost/shared_ptr.hpp>

namespace blink {

template<typename DijkstraState>
class resumable_dijkstra : public dijkstra_state_mixin<DijkstraState>
{
public:
  resumable_dijkstra(const DijkstraState& state) 
    : dijkstra_state_mixin(boost::shared_ptr<DijkstraState>(new DijkstraState(state) ) )
  {}
       
  // Expand the shortest path search until the Interruptor returns true on
  // do_interrupt(). The callback functions of both the FirstVisitor and 
  // SecondVisitor are applied.
  template<typename Interruptor, typename SecondVisitor>
  bool expand(Interruptor interruptor, SecondVisitor visitor)
  {
    typedef joined_visitor<graph_visitor_type, SecondVisitor> visitor_type;
    visitor_type vis(m_graph_visitor, visitor);

    dijkstra_shortest_paths_no_init_at_all(m_graph, m_vertex_predecessor, 
      m_vertex_distance, m_edge_weight, m_vertex_index, m_distance_compare, 
      m_distance_combine, m_distance_zero, vis, m_vertex_color, 
      m_max_priority_queue,interruptor);
      
    return m_max_priority_queue.empty();
  }

  // Expand the shortest path search until the Interruptor return true on 
  // do_interrupt(). The callback functions of the FirstVisitor are applied, 
  // as no second visitor is provided.
  template<typename Interruptor>
  bool expand(Interruptor interruptor)
  {
    return expand(interruptor, boost::default_dijkstra_visitor());
  }

  // Expand the shortest path search exhhaustively. The callback functions of 
  // the FirstVisitor are applied, as no second visitor is provided.
  bool expand()
  {
    return expand(default_interruptor(), boost::default_dijkstra_visitor());
  }

  // Initialize the vertices in the color map, predecessor map and distance
  // map, as well as the FirstVisitor and SecondVisitor
  template<typename SecondVisitor>
  void init_all(SecondVisitor vis)
  {
    typedef joined_visitor<graph_visitor_type, SecondVisitor> visitor_type;
    visitor_type visitor = make_joined_visitor(m_graph_visitor, vis); 
  
    vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(m_graph); ui != ui_end; ++ui) {
      visitor.initialize_vertex(*ui, m_graph);
      put(m_vertex_color, *ui, color_traits::white() );
      put(m_vertex_predecessor, *ui, *ui);
      put(m_vertex_distance, *ui, m_distance_inf);
    }
    clear(m_max_priority_queue);
      //get(max_priority_queue_t()).clear();
  }

  // Set the source Vertex
  template<typename SecondVisitor>
  void put_source(vertex_descriptor source, SecondVisitor vis) 
  {
    typedef joined_visitor<graph_visitor_type, SecondVisitor> visitor_type;
    visitor_type visitor = make_joined_visitor(m_graph_visitor, vis); 

    put(m_vertex_color, source, color_traits::gray() );
    put(m_vertex_distance, source, m_distance_zero);
    m_max_priority_queue.push(source); 
    visitor.discover_vertex(source, m_graph);
  }

  void put_source(vertex_descriptor source) 
  {
    put_source(source, boost::default_dijkstra_visitor());
  }


  // Set multiple source Vertices
  template<typename VerticesRange, typename SecondVisitor>
  void put_sources(const VerticesRange& r, SecondVisitor vis) 
  {
    boost::range_iterator<const VerticesRange>::type i = boost::begin(r);
    const boost::range_iterator<const VerticesRange>::type end = boost::end(r);
    for(; i != end; ++i) {
      put_source(*i, vis);
    }
  }

  // Initialize maps and visitors and set source vertex
  template<typename SecondVisitor>
  void init_from_source(vertex_descriptor source, SecondVisitor vis)
  {
    init_all(vis);
    put_source(source, vis);
  }

  // Initialize maps and first visitor and set source vertex, don't provide 
  // second visitor
  void init_from_source(vertex_descriptor source)
  {
    init_all(boost::default_dijkstra_visitor() );
    put_source(source, boost::default_dijkstra_visitor() );
  }

  // Initialize maps and visitors and multiple source vertices
  template<typename VerticesRange, typename SecondVisitor>
  void init_from_sources(const VerticesRange& r, SecondVisitor vis)
  {
    init_all(vis);
    put_sources(r, vis);
  }

  // Initialize maps and first visitor and multiple source vertices, don't 
  // provide second visitor
  template<typename VerticesRange>
  void init_from_sources(const VerticesRange& r)
  {
    init_all(boost::default_dijkstra_visitor());
    put_sources(r, boost::default_dijkstra_visitor());
  }

};// class resumable_dijkstra

// extends dijkstra_state_helper with the resumable_dijktra type
template<typename Graph, typename Params>
struct resumable_dijkstra_helper 
  : protected dijkstra_state_helper<Graph, Params>
{
  typedef typename dijkstra_state_helper<Graph, Params> parent;
  typedef typename parent::type state_type;

  template<typename Tag>
  struct param : parent::param<Tag>
  {};

  typedef typename resumable_dijkstra<state_type> type;

  static type make(const Graph& g, const Params& params)
  {
    state_type state = parent::make(g, params);
    return type(state);
  }
};

  // Make a resumable_dijkstra object
template <typename Graph, typename Params>
typename resumable_dijkstra_helper<Graph, Params>::type 
  make_resumable_dijkstra(const Graph& g, const Params& params)
{
  return resumable_dijkstra_helper<Graph, Params>::make(g, params); 
}

// Make a resumable_dijkstra, using only default parameters
template <typename Graph>
typename resumable_dijkstra_helper<Graph, boost::no_named_parameters>::type
  make_resumable_dijkstra(const Graph& g)
{
  return make_resumable_dijkstra(g, boost::no_named_parameters() );
}

}// namespace blink

#endif //BLINK_GRAPH_RESUMABLE_DIJKSTRA_HPP
