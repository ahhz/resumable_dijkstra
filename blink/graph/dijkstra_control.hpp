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
// dijkstra_control is used to tell the algorithm object implementations 
// of dijkstra shortest path at which control_points to halt
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_CONTROL_HPP
#define BLINK_GRAPH_DIJKSTRA_CONTROL_HPP

#include <boost/type_traits/integral_constant.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/vector.hpp>

namespace blink {

enum control_point
{
  cp_initialize_vertex,
  cp_examine_vertex,
  cp_examine_edge,
  cp_tree_edge,
  cp_discover_vertex,
  cp_non_tree_edge,
  cp_gray_target,
  cp_black_target,
  cp_finish_vertex,
  cp_edge_relaxed,
  cp_edge_not_relaxed,
  cp_invalid
};

template<typename ControlMap, control_point CP>
struct is_cp_included
{
  typedef typename boost::mpl::at<ControlMap, boost::integral_constant<int, CP> >::type type;
  static const bool value = type::value;
};

template<control_point CP>
struct one_control_point_vector
{
  template<control_point CP2>
  struct is_same_cp
  {
    typedef typename boost::is_same<
      boost::integral_constant<control_point, CP>, 
      boost::integral_constant<control_point, CP2> >::type type;
  };
  typedef typename boost::mpl::vector
    < typename is_same_cp<cp_initialize_vertex>::type
    , typename is_same_cp<cp_examine_vertex>::type
    , typename is_same_cp<cp_examine_edge>::type
    , typename is_same_cp<cp_tree_edge>::type
    , typename is_same_cp<cp_discover_vertex>::type
    , typename is_same_cp<cp_non_tree_edge>::type
    , typename is_same_cp<cp_gray_target>::type
    , typename is_same_cp<cp_black_target>::type
    , typename is_same_cp<cp_finish_vertex>::type
    , typename is_same_cp<cp_edge_relaxed>::type
    , typename is_same_cp<cp_edge_not_relaxed>::type
    >::type type;
};
  
typedef boost::mpl::vector
  < boost::true_type //initialize_vertex
  , boost::true_type //examine_vertex
  , boost::true_type //examine_vertex
  , boost::false_type
  , boost::true_type //discover_vertex
  , boost::false_type
  , boost::false_type
  , boost::false_type
  , boost::true_type //finish_vertex
  , boost::true_type//edge_relaxed
  , boost::true_type//edge_not_relaxed
  >::type dijkstra_visitor_control_point_vector;
 
typedef one_control_point_vector<cp_finish_vertex>::type only_finish_vertex_type;
typedef one_control_point_vector<cp_invalid>::type no_control_point_vector;


template<typename Vector1, typename Vector2>
struct elementwise_or
{
    template<control_point CP>
    struct is_included
    {
      typedef typename is_cp_included<Vector1, CP>::type one;
      typedef typename is_cp_included<Vector2, CP>::type two;
      typedef typename boost::mpl::if_
        <typename  boost::mpl::or_<one, two>::type
        , boost::true_type
        , boost::false_type>::type or;
    };

    typedef typename boost::mpl::vector
    < typename is_included<cp_initialize_vertex>::or
    , typename is_included<cp_examine_vertex>::or
    , typename is_included<cp_examine_edge>::or
    , typename is_included<cp_tree_edge>::or
    , typename is_included<cp_discover_vertex>::or
    , typename is_included<cp_non_tree_edge>::or
    , typename is_included<cp_gray_target>::or
    , typename is_included<cp_black_target>::or
    , typename is_included<cp_finish_vertex>::or
    , typename is_included<cp_edge_relaxed>::or
    , typename is_included<cp_edge_not_relaxed>::or
    >::type type;
};
} //namespace 

#endif //BLINK_GRAPH_DIJKSTRA_CONTROL_HPP