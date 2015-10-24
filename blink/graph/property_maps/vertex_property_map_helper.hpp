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
// The vertex_property_map_helper is used to derive the type and provide a 
// make function for a generic vertex property map.
//
//=======================================================================
//

#ifndef BLINK_VERTEX_PROPERTY_MAP_HELPER_HPP
#define BLINK_VERTEX_PROPERTY_MAP_HELPER_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp> 
#include <boost/type_traits/is_base_and_derived.hpp>

namespace blink
{
  template<typename Value, typename Graph, typename VertexIndexMap, 
    typename Sized = boost::shared_array_property_map<Value, VertexIndexMap>,
    typename Unsized = boost::vector_property_map<Value, VertexIndexMap> >
  struct vertex_property_map_helper
  {
  private:
    typedef Sized sized_map;
	  typedef Unsized unsized_map;

    typedef typename boost::graph_traits<Graph>::traversal_category traversal_category;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	  typedef typename boost::is_base_and_derived<boost::vertex_list_graph_tag
      , traversal_category> known_num;

    BOOST_STATIC_ASSERT((boost::is_same<vertex_descriptor, 
      typename boost::property_traits<VertexIndexMap>::key_type>::value));
    
    static sized_map make(const Graph& g, VertexIndexMap index, boost::true_type)
	  {
		  return sized_map(boost::num_vertices(g), index);
	  }
	  static unsized_map make(const Graph& g, VertexIndexMap index, boost::false_type)
	  {
		  return unsized_map(index);
	  }

  public:
	  typedef typename boost::mpl::if_<known_num, sized_map, unsized_map>::type type;
	  
    static type make(const Graph& g, VertexIndexMap index)
	  {
		  return make(g, index, known_num() );
	  }
	};
}//namespace blink  

#endif // BLINK_VERTEX_PROPERTY_MAP_HELPER_HPP