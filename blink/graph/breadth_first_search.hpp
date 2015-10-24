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
// breadth_first_visit_no_init is a version of the Boost breadth first 
// visit that is interruptable and resumable (because it does not do any
// initialization.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_BREADTH_FIRST_SEARCH_HPP
#define BLINK_GRAPH_BREADTH_FIRST_SEARCH_HPP

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/concept/assert.hpp>

namespace blink {

struct default_interruptor
{
  inline bool do_interrupt() 
  {
    return false;
  }
};

template<class IncidenceGraph, class Buffer, class BFSVisitor
  , class ColorMap, class Interruptor>
void breadth_first_visit_no_init
  (const IncidenceGraph& g, Buffer& Q, BFSVisitor vis, ColorMap color, Interruptor interruptor)
{
  BOOST_CONCEPT_ASSERT(( boost::IncidenceGraphConcept<IncidenceGraph> ));
  typedef boost::graph_traits<IncidenceGraph> GTraits;
  typedef typename GTraits::vertex_descriptor Vertex;
  BOOST_CONCEPT_ASSERT(( boost::BFSVisitorConcept<BFSVisitor, IncidenceGraph> ));
  BOOST_CONCEPT_ASSERT(( boost::ReadWritePropertyMapConcept<ColorMap, Vertex> ));
  typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
  typedef boost::color_traits<ColorValue> Color;
  typename GTraits::out_edge_iterator ei, ei_end;

  while (! Q.empty() && !interruptor.do_interrupt()) {
    Vertex u = Q.top(); Q.pop();            vis.examine_vertex(u, g);
    for (boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
      Vertex v = target(*ei, g);            vis.examine_edge(*ei, g);
      ColorValue v_color = get(color, v);
      if (v_color == Color::white()) {      vis.tree_edge(*ei, g);
        put(color, v, Color::gray());       vis.discover_vertex(v, g);
        Q.push(v);
      } else {                              vis.non_tree_edge(*ei, g);
        if (v_color == Color::gray())       vis.gray_target(*ei, g);
        else                                vis.black_target(*ei, g);
      }
    } // end for
    put(color, u, Color::black());          vis.finish_vertex(u, g);
  } // end while
}

template<class IncidenceGraph, class Buffer, class BFSVisitor, class ColorMap>
void breadth_first_visit_no_init
  (const IncidenceGraph& g, Buffer& Q, BFSVisitor vis, ColorMap color)
{
  breadth_first_visit_no_init(g, Q, vis, color, default_interruptor() );
}

} // namespace blink

#endif // BLINK_GRAPH_BREADTH_FIRST_SEARCH_HPP

