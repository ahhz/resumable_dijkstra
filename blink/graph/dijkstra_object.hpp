//
//=======================================================================
// Copyright 2014
// Author: Alex Hagen-Zanker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// An algorithm object version of the dijkstra shortest path algorithm, It 
// uses the Boost ASIO stackless coroutine as suggested by Dave Abrahams on 
// Boost mailing list. The interface roughly follows the suggestion of  
// Jeremiah Willcock.
//
// The algorithm itself is modified from Boost Graph Library and combines 
// lines from breadth_first_search and bfs_dijkstra_visitor
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_OBJECT_HPP
#define BLINK_GRAPH_DIJKSTRA_OBJECT_HPP

#include <blink/graph/dijkstra_state.hpp>
#include <blink/graph/dijkstra_control.hpp>
#include <blink/graph/relax.hpp>

#include <boost/asio/coroutine.hpp>

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp> //color_traits
#include <boost/property_map/property_map.hpp>
#include <boost/range.hpp>

#include <boost/asio/yield.hpp> // defines macros, undefined at bottom of this file

namespace blink {

template<typename DijkstraState, typename SourceRange, typename ControlMap>
struct dijkstra_object : dijkstra_state_mixin<DijkstraState>
{
  dijkstra_object(const DijkstraState& state, const SourceRange& source_range)
    : dijkstra_state_mixin(boost::shared_ptr<DijkstraState>(new DijkstraState(state) ) )
    , m_source_range(source_range) 
  {}
    
  bool operator()() // jump to next control point
  {
    func();
    return !m_coro.is_complete();
  }

  vertex_descriptor get_u() { return m_u; }          // latest (source) vertex
  vertex_descriptor get_v() { return m_v;}           // latest (target) vertex
  edge_descriptor get_e() { return m_e;}             // latest edge
  control_point get_control_point() { return m_cp; } // latest control point
  
  // TODO (?): do we need two functions to get the latest vertex?
  vertex_descriptor get_vertex()
  {
    switch (m_cp)
    {
    case  cp_initialize_vertex:
    case  cp_examine_vertex:
    case  cp_finish_vertex:
      return m_u;
    case  cp_gray_target:
    case  cp_black_target:
    case  cp_discover_vertex:
      return m_v;
    case  cp_examine_edge:
    case  cp_edge_relaxed:
    case  cp_edge_not_relaxed:
    case  cp_tree_edge:
    case  cp_non_tree_edge:
    case  cp_invalid:
    default:
      // should not be gettting vertex 
      assert(false);
      return m_u;
    }
  }
    
  edge_descriptor get_edge() 
  { 
    return m_e; 
  }            

 private:
  typedef typename typename boost::range_iterator<const SourceRange>::type source_range_iterator;

  template<control_point CP> struct yield_here : is_cp_included<ControlMap, CP> {};
    
  // TODO (?): maybe it is useful return the control_point?
  void func()
  {
      reenter(m_coro) {
        // Init vertices
        for (boost::tie(ui, ui_end) = vertices(m_graph); ui != ui_end; ++ui) {
          m_u = *ui;
          m_v = *ui;
        
          m_graph_visitor.initialize_vertex(m_u, m_graph);
          if(yield_here<cp_initialize_vertex>::value) yield m_cp = cp_initialize_vertex;
        
          put(m_vertex_color, m_u, color_traits::white() );
          put(m_vertex_predecessor, m_u, m_u);
          put(m_vertex_distance, m_u, m_distance_inf);
        }
     
        // Init sources
        clear(m_max_priority_queue);
        ri = boost::begin(m_source_range);
        ri_end = boost::end(m_source_range);
        for(; ri != ri_end; ++ri) {
          put(m_vertex_color, *ri, color_traits::gray() );
          put(m_vertex_distance, *ri, m_distance_zero);
          m_max_priority_queue.push(*ri);
          m_u = *ri;
          m_v = *ri;

          m_graph_visitor.discover_vertex(*ri, m_graph);
          if(yield_here<cp_discover_vertex>::value) yield m_cp = cp_discover_vertex;
        }

        // Main loop
        while (! m_max_priority_queue.empty()) {
          m_u = m_max_priority_queue.top(); 
          m_max_priority_queue.pop();

          m_graph_visitor.examine_vertex(m_u, m_graph);
          if(yield_here<cp_examine_vertex>::value) yield m_cp = cp_examine_vertex;
                
          for (boost::tie(ei, ei_end) = out_edges(m_u, m_graph); ei != ei_end; ++ei) {
            m_e = *ei;
            m_v = target(m_e, m_graph);
          
            if (m_distance_compare( boost::get(m_edge_weight, m_e), m_distance_zero)) {
              boost::throw_exception(boost::negative_edge());
            }
          
            m_graph_visitor.examine_edge(m_e, m_graph);
            if(yield_here<cp_examine_edge>::value) yield m_cp = cp_examine_edge;
             
            v_color = boost::get(m_vertex_color, m_v);
            if (v_color == color_traits::white()) {
         
              if(yield_here<cp_tree_edge>::value) yield m_cp = cp_tree_edge;
                              
              relax_target_confident(m_e, m_graph, m_edge_weight, m_vertex_predecessor, 
                m_vertex_distance, m_distance_combine); 
        
              m_graph_visitor.edge_relaxed(m_e, m_graph);
              if(yield_here<cp_edge_relaxed>::value) yield  m_cp = cp_edge_relaxed;
                           
              m_graph_visitor.discover_vertex(m_v, m_graph);
              if(yield_here<cp_discover_vertex>::value) yield m_cp = cp_discover_vertex;
                    
              put(m_vertex_color, m_v, color_traits::gray());       
              m_max_priority_queue.push(m_v);
          
            } else {
                
              if(yield_here<cp_non_tree_edge>::value) yield m_cp = cp_non_tree_edge;
                       
              if (v_color == color_traits::gray()){
                  
                if(yield_here<cp_gray_target>::value) yield m_cp = cp_gray_target;
              
                decreased = relax_target(m_e, m_graph, m_edge_weight, m_vertex_predecessor, 
                  m_vertex_distance, m_distance_combine, m_distance_compare); 
        
                if (decreased) {
                  m_max_priority_queue.update(m_v);
   
                  m_graph_visitor.edge_relaxed(m_e, m_graph);
                  if(yield_here<cp_edge_relaxed>::value) yield m_cp = cp_edge_relaxed;
                       
                } else {
                  
                  m_graph_visitor.edge_not_relaxed(m_e, m_graph);
                  if(yield_here<cp_edge_not_relaxed>::value) yield  m_cp = cp_edge_not_relaxed;
                       
                }
              } else { // v_color = black
                
                  if(yield_here<cp_black_target>::value) yield m_cp = cp_black_target;
                     
              }
            }
          } // end for
          put(m_vertex_color, m_u, color_traits::black()); 
          m_graph_visitor.finish_vertex(m_u, m_graph);
            
          if(yield_here<cp_finish_vertex>::value) yield m_cp = cp_finish_vertex;
               
        } // end while
        m_cp = cp_invalid;
    } // reenter
  }  // func

  // All variables in the algorithm must be member variables, hence declared here:
  source_range_iterator ri, ri_end;
  vertex_iterator       ui, ui_end;
  out_edge_iterator     ei, ei_end;
  color_type v_color;
  bool decreased;

  // These are the variables that are returned to the user
  vertex_descriptor m_u;
  vertex_descriptor m_v;
  edge_descriptor m_e;
  control_point m_cp;

  // This is input
  const SourceRange& m_source_range;
    
  // The stackless coroutine that allows resuming the algorithm
  boost::asio::coroutine m_coro;
}; // class

  // extend dijkstra_state_helper with dijkstra_object type
template <typename Graph, typename SourceRange, typename ControlMap, typename Params>
struct dijkstra_object_helper : protected dijkstra_state_helper<Graph, Params>
{
  typedef typename boost::bgl_named_params
      < typename Params::value_type
      , typename Params::tag_type
      , typename Params::next_type> params_bgl;
   
  BOOST_STATIC_ASSERT(boost::is_same<Params, params_bgl >::value);

  typedef typename dijkstra_state_helper<Graph, params_bgl> parent;

  template<typename Tag>
  struct param : parent::template param<Tag> 
  { };

  // The dijkstra_state with all template parameters resolved
  typedef typename  parent::type dijkstra_state_type;
 
  typedef typename dijkstra_object<dijkstra_state_type, SourceRange, ControlMap> type;
 
  static type make(
    const Graph& g, SourceRange& source_range, const params_bgl& params)
  {
    return type(parent::make(g, params), source_range);
  }
}; // struct dijkstra_object_helper

// Make a dijkstra_state object
template <typename Graph, typename SourceRange, typename ControlMap, typename Params>
typename dijkstra_object_helper<Graph, SourceRange, ControlMap, Params>::type 
make_dijkstra_object(const Graph& g, SourceRange& source_range, ControlMap, const Params& params)
{
  return dijkstra_object_helper<Graph, SourceRange, ControlMap, Params>::make(g, source_range, params); 
}

// Make a dijkstra_state object, using only default parameters
template <typename Graph, typename SourceRange, typename ControlMap>
typename dijkstra_object_helper<Graph, SourceRange, ControlMap, boost::no_named_parameters>::type 
make_dijkstra_object(const Graph& g, SourceRange& source_range, ControlMap)
{
  return make_dijkstra_object(g, source_range, ControlMap(), boost::no_named_parameters() );
}

// Make a dijkstra_state object, using only default parameters also for control map
// TODO (?): add the control_map to the named parameters
template <typename Graph, typename SourceRange>
typename dijkstra_object_helper<Graph, SourceRange, only_finish_vertex_type, boost::no_named_parameters>::type 
make_dijkstra_object(const Graph& g, SourceRange& source_range)
{
  return make_dijkstra_object(g, source_range, only_finish_vertex_type(), boost::no_named_parameters());
}

template <typename Graph, typename SourceRange, typename Type, typename Tag, typename Rest>
typename dijkstra_object_helper<Graph, SourceRange, only_finish_vertex_type, boost::bgl_named_params<Type, Tag, Rest> >::type
  make_dijkstra_object(const Graph& g, SourceRange& source_range, boost::bgl_named_params<Type, Tag, Rest>& params)
{
    return make_dijkstra_object(g, source_range, only_finish_vertex_type(), params);
}

}// namespace blink

#include <boost/asio/unyield.hpp> // undefines macros
  
    
#endif //BLINK_GRAPH_DIJKSTRA_OBJECT_HPP