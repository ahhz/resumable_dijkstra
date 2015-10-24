//
//=======================================================================
// Copyright 2012-2104
// Author: Alex Hagen-Zanker
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// Demonstrates different ways of using the resumable dijkstra classes
//
//=======================================================================
//
#include <blink/graph/resumable_dijkstra.hpp>
#include <blink/graph/dijkstra_object.hpp>
#include <blink/graph/dijkstra_functions.hpp>
#include <blink/graph/dijkstra_state.hpp>
#include <blink/graph/dijkstra_heap_wrapper.hpp>
#include <blink/graph/dijkstra_visitor/distance_visitor.hpp>
#include <blink/graph/dijkstra_visitor/joined_visitor.hpp>
#include <blink/graph/dijkstra_visitor/logging_visitor.hpp>
#include <blink/graph/dijkstra_visitor/target_visitor.hpp>
#include <blink/graph/dijkstra_visitor/nearest_source_visitor.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/ref.hpp>

#include <iostream>
#include <vector>

template<typename Graph, typename DistanceMap, typename ColorMap>
void report_progress(const Graph& g, DistanceMap d, ColorMap c)
{
  std::cout <<"Vertex" <<'\t' << "Color" <<'\t' << "Distance" << std::endl;
  BGL_FORALL_VERTICES_T(v, g, Graph) {
    std::cout << v <<'\t' << get(c,v) <<'\t' << get(d,v) << std::endl; 
  }
  std::cout << std::endl; 
}

template<typename Graph, typename DistanceMap, typename ColorMap,
  typename NearestMap>
void report_progress(const Graph& g, DistanceMap d, ColorMap c, NearestMap n)
{
  std::cout <<"Vertex" <<'\t' << "Color" <<'\t' << "Distance" << std::endl;
  BGL_FORALL_VERTICES_T(v, g, Graph) {
    std::cout << v <<'\t' << get(c,v) <<'\t' << get(d,v) <<'\t' << get(n,v)
      << std::endl; 
  }
  std::cout << std::endl; 
}

typedef boost::property<boost::edge_weight_t, double> edge_prop;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property, edge_prop>
  graph_type;

typedef boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;
typedef boost::property_map<graph_type, boost::vertex_index_t>::type  vertex_index_map_type;
typedef boost::shared_array_property_map<vertex_descriptor, vertex_index_map_type> 
  predecessor_map_type;

typedef boost::shared_array_property_map<double, vertex_index_map_type> 
  distance_map_type;

typedef boost::two_bit_color_map<vertex_index_map_type> color_map_type;

graph_type make_a_simple_graph(size_t n)
{
  graph_type g(n);
  for(size_t i = 0; i < n; ++i) {
    const vertex_descriptor a = i;
    const vertex_descriptor b = (i + 1) % n;
    const double w = 1.0;
  
    boost::add_edge(a, b, w, g);
    boost::add_edge(b, a, w, g);
  }
  return g;
}

void test_target_visitor(int n)
{
  std::cout << "Test Target Visitor" << std::endl;
  
  typedef blink::target_helper_indirect<graph_type, boost::no_named_parameters>::visitor_type visitor_type;
  typedef blink::resumable_dijkstra_helper<graph_type, boost::no_named_parameters>::type dijkstra_type;

  graph_type g = make_a_simple_graph(n);
  dijkstra_type dijkstra = blink::make_resumable_dijkstra(g);
  
  visitor_type vis = blink::make_target_visitor(dijkstra.get_dijkstra_state());

  vis.add_target(3);
  vis.add_target(6);


  dijkstra.init_from_source(4, vis);

  dijkstra_type::param<boost::vertex_distance_t>::type& distance_map = dijkstra.get(boost::vertex_distance_t() );
  dijkstra_type::param<boost::vertex_color_t>::type& color_map = dijkstra.get(boost::vertex_color_t() );
  
  dijkstra.expand(vis, vis); // vis doubles as interruptor
  std::cout << "Must reach targets 3 and 6" << std::endl;
 
  report_progress(g, distance_map, color_map);

  vis.add_target(5);
  dijkstra.expand(vis, vis); // vis doubles as interruptor
  std::cout << "Must reach target 5 too" << std::endl;
  report_progress(g, distance_map, color_map);

  vis.add_target(8);
  dijkstra.expand(vis, vis); // vis doubles as interruptor
  std::cout << "Must reach target 8 too" << std::endl;
  report_progress(g, distance_map, color_map);
  
  dijkstra.expand();
  std::cout << "Must reach all targets" << std::endl;
  report_progress(g, distance_map, color_map);
}

void test_target_visitor_auto(int n)
{
  std::cout << "Test Target Visitor" << std::endl;
  
 
  auto g = make_a_simple_graph(n);
 

  auto dijkstra = blink::make_resumable_dijkstra(g);
  auto vis = blink::make_target_visitor(dijkstra.get_dijkstra_state());

  vis.add_target(3);
  vis.add_target(6);
  
  dijkstra.init_from_source(4, vis);
  dijkstra.expand(vis,vis ); // vis doubles as interruptor
  
  auto distance_map = dijkstra.get(boost::vertex_distance_t());
  auto color_map = dijkstra.get(boost::vertex_color_t());

  report_progress(g, distance_map, color_map);
  
  vis.add_target(5);
  dijkstra.expand(vis, vis); // vis doubles as interruptor
  report_progress(g, distance_map, color_map);
 
  vis.add_target(8);
  dijkstra.expand(vis, vis); // vis doubles as interruptor
  report_progress(g, distance_map, color_map);
   
  dijkstra.expand();
  report_progress(g, distance_map, color_map);
 }

void test_target_visitor_convenience(int n)
{
  std::cout << "Test Target Visitor - Convenience" << std::endl;
 
  graph_type g = make_a_simple_graph(n);
  distance_map_type distance(n, get(boost::vertex_index_t(), g) );
  color_map_type color(n, get(boost::vertex_index_t(), g) );
  
  std::vector<int> targets;
  targets.push_back(5);
  targets.push_back(6);
  
  vertex_descriptor orig = 4;

  blink::dijkstra_shortest_path_targets(g, orig, targets, 
    distance_map(distance).color_map(color) );

  report_progress(g, distance, color);
}

void test_nearest_visitor(int n)
{
  std::cout << "Test Nearest Visitor" << std::endl;
  typedef blink::nearest_source_helper_indirect<graph_type, boost::no_named_parameters>::visitor_type visitor_type;
  typedef blink::resumable_dijkstra_helper<graph_type, boost::no_named_parameters>::type dijkstra_type; 

  graph_type g = make_a_simple_graph(n);
  dijkstra_type dijkstra = blink::make_resumable_dijkstra(g);
  visitor_type vis = blink::make_nearest_source_visitor(dijkstra.get_dijkstra_state());

  std::vector<vertex_descriptor> sources;

  sources.push_back(4);
  sources.push_back(7);
  sources.push_back(11);

  dijkstra.init_from_sources(sources, vis);
  dijkstra.expand(blink::default_interruptor(), vis);

  dijkstra_type::param<boost::vertex_distance_t>::type  distance_map 
    = dijkstra.get(boost::vertex_distance_t() );
  dijkstra_type::param<boost::vertex_color_t>::type  color_map 
    = dijkstra.get(boost::vertex_color_t() );
  
  report_progress(g, distance_map, color_map, vis.m_nearest_source_map);
}

void test_nearest_visitor_convenience(int n)
{
  std::cout << "Test Nearest Visitor - Convenience Function" << std::endl;
  
  graph_type g = make_a_simple_graph(n);

  typedef blink::dijkstra_state_helper<graph_type, boost::no_named_parameters>::type state_type;
  typedef blink::nearest_source_helper<state_type>::visitor_type visitor_type;
  typedef blink::nearest_source_helper<state_type>::map_type map_type;
  
  std::vector<vertex_descriptor> sources;

  sources.push_back(4);
  sources.push_back(7);
  sources.push_back(11);

  std::pair<state_type, visitor_type> result = blink::dijkstra_shortest_path_nearest_source(g, sources);

  state_type::param<boost::vertex_distance_t>::type distance_map 
    = result.first.get<boost::vertex_distance_t>();

  state_type::param<boost::vertex_color_t>::type color_map 
    = result.first.get<boost::vertex_color_t>();
    
  report_progress(g, distance_map, color_map, result.second.m_nearest_source_map);
}

void test_distance_visitor(int n)
{
  typedef blink::resumable_dijkstra_helper<graph_type, boost::no_named_parameters>::type dijkstra_type;
  typedef blink::distance_visitor_helper_indirect<graph_type, boost::no_named_parameters> helper;
  typedef helper::type visitor_type;
  
  std::cout << "Test Distance Visitor" << std::endl;
  graph_type g = make_a_simple_graph(n);
 
  double first_target_distance = 2.0;
  double second_target_distance = 4.0;
 
  dijkstra_type dijkstra = blink::make_resumable_dijkstra(g);
  visitor_type visitor = helper::make(dijkstra.get_dijkstra_state(), first_target_distance);
 
  vertex_descriptor orig = 4;
  dijkstra.init_from_source(orig, visitor);
  dijkstra.expand(visitor, visitor); // vis doubles as interrupetor
   
  dijkstra_type::param<boost::vertex_distance_t>::type  distance_map 
    = dijkstra.get(boost::vertex_distance_t() );

  dijkstra_type::param<boost::vertex_color_t>::type  color_map 
    = dijkstra.get(boost::vertex_color_t() );

  std::cout << "must reach distance 2" << std::endl;
  report_progress(g, distance_map, color_map);

  visitor_type second_visitor = helper::make(dijkstra.get_dijkstra_state(), second_target_distance);
  
  dijkstra.expand(second_visitor, second_visitor); // now secondvis is the interruptor
  std::cout << "must reach distance 4" << std::endl;
  report_progress(g, distance_map, color_map);
}


void test_classic_boost(int n)
{
  std::cout << "Test Classic Boost(note the faulty colormap)" << std::endl;
 
  graph_type g = make_a_simple_graph(n);
  distance_map_type distance(n, get(boost::vertex_index, g));
  color_map_type color(n, get(boost::vertex_index, g) );
  vertex_descriptor orig = 4;
  boost::dijkstra_shortest_paths(g, orig, distance_map(distance).color_map(color));
 
  report_progress(g, distance, color);
}


void test_classic_blink(int n)
{
  std::cout << "Test Classic Blink" << std::endl;
 
  graph_type g = make_a_simple_graph(n);
  typedef blink::dijkstra_state_helper<graph_type, boost::no_named_parameters>::type state_type;
  vertex_descriptor orig = 4;
  state_type state = blink::dijkstra_shortest_path_plain(g, orig);

  report_progress(g, state.get<boost::vertex_distance_t>(), state.get<boost::vertex_color_t>());
}


void test_distance_visitor_convenience(int n)
{
  std::cout << "Test Distance Visitor - Convenience" << std::endl;
  graph_type g = make_a_simple_graph(n);
  distance_map_type distance(n, get(boost::vertex_index, g));
  color_map_type color(n, get(boost::vertex_index, g) );
 
  double target_distance = 4;
  vertex_descriptor orig = 4;
  
  blink::dijkstra_shortest_path_distance(g, orig, target_distance, distance_map(distance).color_map(color));

  std::cout << "must reach distance 4" << std::endl;
  report_progress(g, distance, color );
}

void test_dijkstra_object(int n)
{
  std::cout << "Test Dijkstra Object" << std::endl;
  graph_type g = make_a_simple_graph(n);
  
  typedef std::vector<vertex_descriptor> source_range_type; 
  typedef blink::only_finish_vertex_type control_map_type;
  typedef boost::no_named_parameters parameters_type;
  typedef blink::dijkstra_object_helper<
      graph_type, 
      source_range_type, 
      control_map_type, 
      parameters_type>::type dijkstra_object_type;

  double target_distance = 4;
  vertex_descriptor orig = 4;
  source_range_type sources(1,orig);

  dijkstra_object_type dijkstra = blink::make_dijkstra_object(g, sources, blink::only_finish_vertex_type());
 
  dijkstra_object_type::param<boost::vertex_distance_t>::type  distance_map = dijkstra.get(boost::vertex_distance_t() );
  dijkstra_object_type::param<boost::vertex_color_t   >::type  color_map    = dijkstra.get(boost::vertex_color_t()    );

  while(dijkstra()) {
    const double distance = get(distance_map, dijkstra.get_u());
    if(distance >= target_distance) {
      break;
    }
  }
  std::cout << "must reach distance " << target_distance << std::endl;
  report_progress(g, distance_map, color_map );
}

void test_dijkstra_object2(int n)
{
  std::cout << "Test Dijkstra Object - yielding at all visitor positions" << std::endl;
  std::cout << "Every action should be logged twice once by the object and once by the object user" << std::endl;
  
  typedef blink::logging_visitor<std::ostream> visitor_type;
  typedef boost::bgl_named_params<visitor_type, boost::graph_visitor_t> parameters_type;
  typedef std::vector<vertex_descriptor> source_range_type; 
  typedef blink::dijkstra_visitor_control_point_vector control_map_type;
  
  typedef blink::dijkstra_object_helper<
    graph_type, 
    source_range_type, 
    control_map_type, 
    parameters_type>::type dijkstra_object_type;
  
  graph_type g = make_a_simple_graph(n);
  visitor_type vis(std::cout);

  vertex_descriptor orig = 4;
  source_range_type sources(1,orig);

  dijkstra_object_type dijkstra = blink::make_dijkstra_object(g, sources, control_map_type(), boost::visitor(vis) );
  
  while(dijkstra()) {
    switch(dijkstra.get_control_point())
    {
    case blink::cp_initialize_vertex: 
      vis.initialize_vertex(dijkstra.get_v(), g);
      break;
    case blink::cp_examine_vertex: 
      vis.examine_vertex(dijkstra.get_u(), g); 
      break;
    case blink::cp_examine_edge: 
      vis.examine_edge(dijkstra.get_e(), g); 
      break;
    case blink::cp_discover_vertex: 
      vis.discover_vertex(dijkstra.get_v(), g); 
      break;
    case blink::cp_edge_relaxed: 
      vis.edge_relaxed(dijkstra.get_e(), g); 
      break;
    case blink::cp_edge_not_relaxed: 
      vis.edge_not_relaxed(dijkstra.get_e(), g); 
      break;
    case blink::cp_finish_vertex: 
      vis.finish_vertex(dijkstra.get_u(), g); 
      break;
    default : std::cout << "other control_point" << std::endl;
    }
  }
}

void test_with_boost_heap(int n)
{
  typedef boost::indirect_cmp<distance_map_type, std::greater<double> > indirect_compare;
  typedef boost::heap::d_ary_heap
    < vertex_descriptor
    , boost::heap::compare<indirect_compare>
    , boost::heap::mutable_<true>
    , boost::heap::arity<4>
    , boost::heap::store_parent_pointer<false>
    , boost::heap::constant_time_size<false>
    > heap_type;

  typedef blink::dijkstra_heap_wrapper<heap_type, graph_type, vertex_index_map_type> wrapped_heap_type;
  typedef boost::bgl_named_params<boost::reference_wrapper<wrapped_heap_type>, boost::max_priority_queue_t,
    boost::bgl_named_params<distance_map_type, boost::vertex_distance_t> > parameters_type;
  typedef blink::dijkstra_state_helper<graph_type, parameters_type>::type state_type;
  
  graph_type g = make_a_simple_graph(n);
  vertex_index_map_type index = get(boost::vertex_index, g);
  distance_map_type distance(n, index);
  vertex_descriptor orig = 4;
  wrapped_heap_type heap(g, index, distance);
  state_type state = blink::dijkstra_shortest_path_plain(g, orig,
    boost::max_priority_queue(heap).distance_map(distance));
  report_progress(g, state.get<boost::vertex_distance_t>(), state.get<boost::vertex_color_t>());
}

void test_dijkstra_object3(int n)
{
  std::cout << "Test Dijkstra Object - Simplest Demo" << std::endl;

  graph_type g = make_a_simple_graph(n);
  vertex_descriptor orig = 4;
  std::vector<vertex_descriptor> sources(1, orig);
 
  auto dijkstra = blink::make_dijkstra_object(g, sources);

  while (dijkstra()) {
    auto v = dijkstra.get_vertex();
    auto distance_map = dijkstra.get<boost::vertex_distance_t>();
    if (get(distance_map, v) >= 2) break;
  }
  report_progress(g, dijkstra.get<boost::vertex_distance_t>(), dijkstra.get<boost::vertex_color_t>());
}

int main() 
{
  int n = 12;
 
  test_target_visitor(n);
  test_target_visitor_convenience(n);
 
  test_distance_visitor(n);
  test_distance_visitor_convenience(n);
  
  test_classic_boost(n);
  test_classic_blink(n);
  
  test_nearest_visitor(n);
  test_nearest_visitor_convenience(n);
  
  test_dijkstra_object(n);
  test_dijkstra_object2(n);
  test_dijkstra_object3(n);
  test_with_boost_heap(n);

  return 0;
}