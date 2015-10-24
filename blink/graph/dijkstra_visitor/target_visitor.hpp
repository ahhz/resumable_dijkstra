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
// The target visitor implements the dijkstra visitor and interruptor 
// concepts. It returns true on do_interrupt once a desired set of target
// vertices is finalized (black)
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_TARGET_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_TARGET_VISITOR_HPP

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace blink{

template<typename Graph, typename ColorMap>
class target_visitor : public boost::default_dijkstra_visitor
{
public:
  typedef typename boost::property_traits<ColorMap>::value_type color_type;
  typedef typename boost::property_traits<ColorMap>::key_type vertex_descriptor;
  typedef boost::color_traits<color_type> color_traits;
  
  // m_progress_map keeps track of targets, it is not the same colormap 
  // as used by the dijkstra shortest path algorithm 
  // color_traits::white() not found and not required
  // color_traits::gray()  not found but required
  // color_traits::black() found 

   target_visitor(const Graph& graph, ColorMap progress_map)  
    : m_progress_map(progress_map), m_graph(&graph)
  {
    init_count();
  }

  void init_progress_map()
  {
    typename boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(*m_graph); ui != ui_end; ++ui) {
       put(m_progress_map, *ui, color_traits::white() );
    }
  }

  void init_count()
  {
    m_num_white.reset(new size_t(num_vertices(*m_graph) ) );
    m_num_gray.reset(new size_t(0) );
  }
  
  template<typename U, typename G>
  inline void finish_vertex(const U& u, const G& g)
  {
    color_type& c = m_progress_map[u];
    if(c == color_traits::white() ) {
      c = color_traits::black();
      (*m_num_white)--;
    } else if(c == color_traits::gray() ) {
      c = color_traits::black();
      (*m_num_gray)--;
    } // else black{ do nothing }
  }

  inline bool do_interrupt() const {
    return (*m_num_gray) == 0;
  }

  void add_target(const vertex_descriptor& v)
  {
    if( get(m_progress_map,v) == color_traits::white() ) {
      put(m_progress_map, v, color_traits::gray() );
      (*m_num_white)--;
      (*m_num_gray)++;
    }
  }
  template<typename TargetRange>
  void add_targets(const TargetRange& r) 
  {
    boost::range_iterator<const TargetRange>::type i = boost::begin(r);
    boost::range_iterator<const TargetRange>::type end = boost::end(r);
    for(; i != end; ++i) {
      add_target(*i);
    }
  }

  const Graph* m_graph;
  ColorMap m_progress_map;
  boost::shared_ptr<size_t> m_num_white;
  boost::shared_ptr<size_t> m_num_gray;
};


template<typename DijkstraState>
struct target_helper
{
  typedef typename DijkstraState::template param<boost::vertex_index_t>::type index_type;
  typedef typename DijkstraState::graph_type graph_type;

  typedef typename boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
  typedef typename vertex_property_map_helper<boost::default_color_type, graph_type, index_type> helper; 
  typedef typename helper::type map_type;
  typedef typename target_visitor<graph_type, map_type> visitor_type;

  static map_type make_map(DijkstraState& state)
  {
    return helper::make(state.get_graph(), state.get<boost::vertex_index_t>());
  }
  
  static visitor_type make_visitor(DijkstraState& state)
  {
    map_type map = make_map(state);
    visitor_type visitor(state.get_graph(), map);
    visitor.init_count();
    visitor.init_progress_map();
      
    return visitor;
  }
};

template<typename Graph, typename Params>
struct target_helper_indirect : target_helper<typename dijkstra_state_helper<Graph, Params>::type>
{};

template<typename DijkstraState>
typename typename target_helper<DijkstraState>::visitor_type
make_target_visitor(DijkstraState& state)
{
  return target_helper<DijkstraState>::make_visitor(state);
}

} // namespace boost;

#endif //BLINK_GRAPH_DIJKSTRA_VISITOR_TARGET_VISITOR_HPP