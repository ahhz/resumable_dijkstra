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
// distance_visitor implements the dijkstra visitor and interruptor 
// concepts. It returns true on do_interrupt() once all vertices within a
// given distance are found.
//
//=======================================================================
//
#ifndef BLINK_GRAPH_DIJKSTRA_VISITOR_DISTANCE_VISITOR_HPP
#define BLINK_GRAPH_DIJKSTRA_VISITOR_DISTANCE_VISITOR_HPP

#include <boost/graph/dijkstra_shortest_paths.hpp> // default_dijkstra_visitor
#include <boost/property_map/property_map.hpp>
#include <boost/smart_ptr.hpp>

#include <functional> //std::less

namespace blink {

template<typename DistanceMap, typename Compare = 
  std::less<typename boost::property_traits<DistanceMap>::value_type > >
class distance_visitor : public boost::default_dijkstra_visitor
{
  typedef typename boost::property_traits<DistanceMap>::value_type distance_type;

public:
  distance_visitor(DistanceMap distance_map, distance_type target_distance, 
    const Compare& compare = Compare()) 
    : m_distance_map(distance_map), m_target_distance(target_distance), 
    m_compare(compare)
  {
    m_do_interrupt.reset(new bool(false) );
  }
  
  template<typename U, typename G>
  void finish_vertex(const U& u, const G& g)
  {
    if( !m_compare(get(m_distance_map,u), m_target_distance) ) { 
      *m_do_interrupt = true;
    }
  }

  inline bool do_interrupt() const 
  {
    return *m_do_interrupt;
  }

private:
  boost::shared_ptr<bool> m_do_interrupt;
  DistanceMap m_distance_map; 
  distance_type m_target_distance;
  Compare m_compare;
};

template<typename DijkstraState>
struct distance_visitor_helper
{
  typedef typename DijkstraState::template param<boost::distance_compare_t>::type compare_type;
  typedef typename DijkstraState::template param<boost::vertex_distance_t>::type distance_map_type;
  typedef typename DijkstraState::template param<boost::vertex_index_t>::type index_type;
  typedef typename DijkstraState::template param<boost::distance_inf_t>::type distance_value_type;
  typedef typename distance_visitor<distance_map_type, compare_type> type;

  static type make(DijkstraState& state, distance_value_type d)
  {
    return type(state.get<boost::vertex_distance_t>(), d
      , state.get<boost::distance_compare_t>());
  }
};

template<typename Graph, typename Params>
struct distance_visitor_helper_indirect 
  : distance_visitor_helper<typename dijkstra_state_helper<Graph, Params>::type>
{};
 
template <typename DijkstraState, typename DistanceValue>
typename distance_visitor_helper<DijkstraState>::type
make_distance_visitor(DijkstraState& state, DistanceValue d)
{
  return distance_visitor_helper<DijkstraState>::make(state, d);
}


}; // namespace blink;

#endif //BLINK_GRAPH_DIJKSTRA_VISITOR_DISTANCE_VISITOR_HPP