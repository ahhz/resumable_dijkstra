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
// dijkstra_paremeter_helper is used to unpack the parameters of the (old)
// bgl_named_parameters and where no parameters are provided make the 
// defaults.
//
//=======================================================================
//

#ifndef BLINK_GRAPH_DIJKSTRA_PARAMETER_HELPER_HPP
#define BLINK_GRAPH_DIJKSTRA_PARAMETER_HELPER_HPP

#include <blink/graph/dijkstra_queue.hpp>
#include <blink/graph/property_maps/vertex_property_map_helper.hpp>

#include <boost/graph/dijkstra_shortest_paths.hpp> //default_dijkstra_visitor
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/two_bit_color_map.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp> 
#include <boost/type_traits/conditional.hpp>

#include <functional> //std::less, std::plus
#include <limits> //std::numeric_limits<type>::max()

namespace blink  {

struct stored_ref_tag{};
struct stored_ptr_tag{};
struct stored_val_tag{};

template<typename T>
struct is_shared_ptr // either boost or std
{
private:
  // Types "yes" and "no" are guaranteed to have different sizes,
  // specifically sizeof(yes) == 1 and sizeof(no) == 2.
  typedef char yes[1];
  typedef char no[2];
 
  template <typename C>
  static yes& has_element_type(typename C::element_type*);
 
  template <typename>
  static no& has_element_type(...);
  
  // If the "sizeof" the result of calling test<T>(0) would be equal to the sizeof(yes),
  // the first overload worked and T has a nested type named foobar.
  static const bool has_element = sizeof(has_element_type<T>(0)) == sizeof(yes);

  template<typename Ptr, bool HasElement>
  struct is_ptr
  { };

  template<typename Ptr>
  struct is_ptr<Ptr,true>
  {
    typedef typename boost::is_same<boost::shared_ptr<typename Ptr::element_type>, Ptr>::type boost_type;
    typedef typename boost::is_same<std::shared_ptr<typename Ptr::element_type>, Ptr>::type std_type;
    typedef typename boost::mpl::or_<boost_type, std_type>::type type;
    static const bool value = type::value;
    typedef typename Ptr::element_type unwrapped;
  };

  template<typename Ptr>
  struct is_ptr<Ptr,false>
  {
    static const bool value = false;
    typedef boost::false_type type;
    typedef Ptr unwrapped;
  };
public:
  static const bool value = is_ptr<T, has_element>::value;
  typedef typename is_ptr<T, has_element>::type type;
  typedef typename is_ptr<T, has_element>::unwrapped unwrapped;
};

// Only used for max_priority_queue to determine whether it is wrapped in a 
// reference or a shared_pointer
template<typename RawType>
struct processed_type
{
  typedef RawType stored_type;

  typedef typename boost::conditional
        < boost::is_reference_wrapper<RawType>::value
        , stored_ref_tag, typename boost::mpl::if_
          < typename is_shared_ptr<RawType>::type
          , stored_ptr_tag
          , stored_val_tag>::type
        >::type stored_type_tag;
    
  typedef typename boost::conditional
      < boost::is_reference_wrapper<RawType>::value
      , typename boost::unwrap_reference<RawType>::type
      , typename boost::mpl::if_
        < typename is_shared_ptr<RawType>::type
        , typename is_shared_ptr<RawType>::unwrapped
        , RawType>::type
      >::type type;
};

// This struct derives all the parameters from the parameter pack, 
// supplying defaults where necessary.
template<typename Graph, typename Params>
struct dijkstra_parameter_helper
{
  typedef typename boost::bgl_named_params
      < typename Params::value_type
      , typename Params::tag_type
      , typename Params::next_type> bgl_params;

  typedef typename boost::is_same<bgl_params, Params>::type is_bgl_named_params;
        
  BOOST_STATIC_ASSERT(is_bgl_named_params::value);

  typedef typename dijkstra_parameter_helper<Graph, Params> this_type;
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef boost::is_base_and_derived<boost::vertex_list_graph_tag,
    typename boost::graph_traits<Graph>::traversal_category>
    known_num_vertices;

  // this struct has to be specialized, for all parameters (Tags) that 
  // require a default.
  template<typename Tag, typename Dummy> 
  struct default_param 
  {
    struct has_no_default
    {};

    typedef has_no_default type;
//      static type make(){  return has_no_default; }
  };

  
  // find the parameter type for a Tag in the parameter pack
  template<typename Tag> 
  struct given_param 
  {
    typedef typename boost::get_param_type<Tag, Params>::type stored_type;
    typedef typename stored_val_tag stored_type_tag;
    typedef typename stored_type type;
  };

  // find the parameter type for a Tag in the parameter pack
  // the bgl library put a reference wrapper around the max_priority_queue_t parameter.
  // make sure this is accounted for
  template<> 
    struct given_param<boost::max_priority_queue_t> 
  {
    typedef typename boost::get_param_type<boost::max_priority_queue_t, Params>::type stored_type;
    typedef typename boost::unwrap_reference<stored_type>::type type;
    typedef stored_ref_tag stored_type_tag;
  };


  // check if the parameter is in the parameter pack
  template<typename Tag> 
  struct is_given 
  {
    typedef typename boost::is_same< typename given_param<Tag>::type, 
      boost::param_not_found>::type is_not_given;
    typedef typename boost::is_same<boost::false_type, is_not_given>::type type;
  };

  // use the parameters in the pack, if it is not there, use default
    template <typename Tag, bool IsGiven>
  struct get_type
  {

  };

  template <typename Tag>
  struct get_type<Tag, false>
  {
    typedef typename default_param<Tag, void>::type type;
    typedef typename default_param<Tag, void>::stored_type stored_type;
    typedef typename default_param<Tag, void>::stored_type_tag stored_type_tag;
  };

  template <typename Tag>
  struct get_type<Tag, true>
  {
    typedef typename given_param<Tag>::type type;
    typedef typename given_param<Tag>::stored_type stored_type;
    typedef typename given_param<Tag>::stored_type_tag stored_type_tag;

  };


  template<typename Tag> 
  struct param 
  {
    typedef typename get_type<Tag, is_given<Tag>::type::value>::type type;
    typedef typename get_type<Tag, is_given<Tag>::type::value>::stored_type stored_type;
    typedef typename get_type<Tag, is_given<Tag>::type::value>::stored_type_tag stored_type_tag;
    /*
    boost::mpl::eval_if<
      typename is_given<Tag>::type, 
      typename get_given_type<Tag>,
      typename get_default_type<Tag> >::type type;

    typedef typename boost::mpl::eval_if<
      typename is_given<Tag>::type, 
      typename get_given_stored_type<Tag>,
      typename get_default_stored_type<Tag> >::type stored_type;

    typedef typename boost::mpl::eval_if<
      typename is_given<Tag>::type,
      typename get_given_stored_type_tag<Tag>,
      typename get_default_stored_type_tag<Tag> >::type stored_type_tag;
      */
//    typedef typename boost::mpl::if_<
//      typename is_given<Tag>::type, 
//      typename given_param<Tag>::stored_type,
//      typename default_param<Tag>::stored_type>::type stored_type;

//    typedef typename boost::mpl::if_<
//      typename is_given<Tag>::type, 
//      typename given_param<Tag>::stored_type_tag,
//      typename default_param<Tag>::stored_type_tag>::type stored_type_tag;
  };


  // Overriding specific parameters:
  // The order is relevant because some parameters depend on each other
    
  template<typename Tag>
  struct has_edge_property
  {
    typedef typename boost::edge_property_type<Graph>::type edge_properties;
    typedef typename boost::is_same<typename boost::property_value<edge_properties, Tag >::type, void>::type not_exist_type;
    const static bool value = !not_exist_type::value;
  };

  template<typename Tag>
  struct has_vertex_property
  {
    typedef typename boost::vertex_property_type<Graph>::type vertex_properties;
    typedef typename boost::is_same<typename boost::property_value<vertex_properties, Tag>::type, void>::type not_exist_type;
    const static bool value = !not_exist_type::value;
  };


  // Use the internal property map as default for vertex_index_t
  template<typename Dummy>
  struct default_param<boost::vertex_index_t, Dummy>
  {
    // Check not appropriate here? Because identity map is default? 

    //const static bool check = has_vertex_property<boost::vertex_index_t>::value;
    //static_assert(check, "reading internal property from graph without such property");
    
    typedef typename boost::property_map<Graph, boost::vertex_index_t >::const_type type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;
    static type make(const Graph& g) 
    { 
      return get(boost::vertex_index_t(), g); 
    }
  };

  template<typename Dummy> 
  struct default_param<boost::edge_weight_t, Dummy>
  {
    const static bool check = has_edge_property<boost::edge_weight_t>::value;
    static_assert(check, "reading internal property from graph without such property");
    typedef typename boost::property_map<Graph, boost::edge_weight_t>::const_type type;
 
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;
    static type make(const Graph& g) 
    { 
      return get(boost::edge_weight_t(), g); 
    }
  };

  typedef typename param<boost::vertex_index_t>::type vertex_index_map_type;
  typedef typename param<boost::edge_weight_t>::type weight_map_type;
    
  // Derive the type for distance_value (not yet distance map)
  // if the distance_map is given, then use its value_type
  // else use the value_type of the weight_map_type map
      
  typedef typename 
    boost::property_traits<
      typename boost::mpl::if_
        < typename is_given<boost::vertex_distance_t>::type
        , typename given_param<boost::vertex_distance_t>::type
        , weight_map_type>::type
      >::value_type distance_value_type;  


  // use shared_array_property_map as default for vertex_distance_t
  // if num_vertices is not known, use vector_property_map
  template<typename Dummy>
  struct default_param<boost::vertex_distance_t, Dummy>
  {
    typedef vertex_property_map_helper<distance_value_type, Graph
      , vertex_index_map_type> helper;
      
    typedef typename helper::type type;

    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make(const Graph& g, vertex_index_map_type i) 
    { 
      return helper::make(g, i);
    }
  };

  // the vertex_predecessor_t is not strictly necessary, hence the default 
  // is null_property_map
  template<typename Dummy>
  struct default_param<boost::vertex_predecessor_t, Dummy>
  {
    typedef typename boost::null_property_map<vertex_descriptor, vertex_descriptor> type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make()
    {
      return type();
    }
  };

  // use two_bit_color_map as default for vertex_color_t
  // if num_vertices is not known, use vector_property_map
  template<typename Dummy>
  struct default_param<boost::vertex_color_t, Dummy>
  {
    typedef boost::two_bit_color_map<vertex_index_map_type> sized_type;

    typedef  vertex_property_map_helper<boost::two_bit_color_type, Graph
      , vertex_index_map_type, sized_type> helper;
      
    typedef typename helper::type type;

    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make(const Graph& g, vertex_index_map_type i) 
    { 
      return helper::make(g, i);
    }
  };

  // use std::less as default for distance_compare_t
  template<typename Dummy>
  struct default_param<boost::distance_compare_t, Dummy>
  {
    typedef std::less<distance_value_type> type;

    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make() 
    { 
      return type(); 
    }
  };

  // relies on the following patch being used: https://svn.boost.org/trac/boost/attachment/ticket/7387/minor_improvements.patch
  // for now use own relax.hpp
  template<typename Dummy>
  struct default_param<boost::distance_combine_t, Dummy>
  {
    // typedef boost::closed_plus<distance_value_type> type;// use this if boost::closed_plus needs to be used
    typedef std::plus<distance_value_type> type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make(const distance_value_type& inf) // TODO: get rid of inf
    {  
        //return type(inf);  // use this if boost::closed_plus needs to be used
        return type(); // use this when std::plus is used
    }
  };

  // use std::numeric_limits<Distance>::max for default of distance_zero_t
  template<typename Dummy>
  struct default_param<boost::distance_inf_t, Dummy>
  {
    typedef distance_value_type type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make() 
    { 
      return std::numeric_limits<type>::max(); 
    }
  };

  // use zero initializer for default of distance_zero_t
  template<typename Dummy>
  struct default_param<boost::distance_zero_t, Dummy>
  {
    typedef distance_value_type type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make() 
    { 
      return type(); 
    }
  };

  // use default_dijkstra_visitor for default of graph_visitor_t
  template<typename Dummy>
  struct default_param<boost::graph_visitor_t, Dummy>
  {
    typedef boost::default_dijkstra_visitor type;
    typedef type stored_type;
    typedef stored_val_tag stored_type_tag;

    static type make() 
    { 
      return boost::default_dijkstra_visitor(); 
    }
  };

  // the default for the priority queue
  template<typename Dummy>
  struct default_param<boost::max_priority_queue_t, Dummy> 
  {
    typedef dijkstra_queue_bgl< Graph
      , typename param<boost::vertex_distance_t>::type
      , typename param<boost::vertex_index_t>::type
      , typename param<boost::distance_compare_t>::type > traits;
    typedef typename traits::type type;
      
    typedef boost::shared_ptr<type> stored_type;
    typedef stored_ptr_tag stored_type_tag;

    static boost::shared_ptr<type> make(const Graph& g, 
      typename param<boost::vertex_distance_t>::type d,
      typename param<boost::distance_compare_t>::type c,
      typename param<boost::vertex_index_t>::type i)
    {
      return traits::make_smart(g, d, c, i);
    }
  };
       
  // 5 * 2 overloads to get the parameter either from the parameter pack or 
  // from the default_param::make function

  // Originally this part of the code was more compact by using boost::bind to 
  // get rid of the a, b, c, d arguments. However, this was confusing the
  // compiler and failing (VS2010) when a default type had an implied default 
  // constructor (i.e. it got a substitution failure that IS an error related 
  // for the make() of the preprocessor_map (null_property_map)

  template<typename Tag, typename A, typename B, typename C, 
    typename D>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params& params,
    const A&, const B&, const C&, const D&, boost::true_type)
  {
    return get_param(params, tag);
  }

  template<typename Tag, typename A, typename B, typename C, 
    typename D>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params&,
    const A& a, const B& b, const C& c, const D& d, boost::false_type)
  {
    return default_param<Tag, void>::make(a, b, c, d);
  }

  template<typename Tag, typename Params, typename A, typename B, typename C>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params& params,
    const A&, const B&, const C&, boost::true_type)
  {
    return get_param(params, tag);
  }

  template<typename Tag, typename A, typename B, typename C>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params&,
    const A& a, const B& b, const C& c, boost::false_type)
  {
    return default_param<Tag, void>::make(a, b, c);
  }

  template<typename Tag, typename A, typename B>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params& params,
    const A&, const B&, boost::true_type)
  {
    return get_param(params, tag);
  }

  template<typename Tag, typename A, typename B>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params&,
    const A& a, const B& b, boost::false_type)
  {
    return default_param<Tag, void>::make(a, b);
  }

  template<typename Tag, typename A>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params& params,
    const A&, boost::true_type)
  {
    return get_param(params, tag);
  }

  template<typename Tag, typename A>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params&,
    const A& a, boost::false_type)
  {
    return default_param<Tag, void>::make(a);
  }

  template<typename Tag>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params& params,
    boost::true_type)
  {
    return get_param(params, tag);
  }

  template<typename Tag>
  static typename param<Tag>::stored_type make_helper(Tag tag, const Params&,
    boost::false_type)
  {
    return default_param<Tag, void>::make();
  }

    // 5 overloads of functions to read parameters using default_param::make
  template<typename Tag>
  static typename param<Tag>::stored_type make(Tag tag, const Params& params)
  {
    typedef typename is_given<Tag>::type is_given;
    return make_helper(tag, params, is_given());
  }

  template<typename Tag, typename A>
  static typename param<Tag>::stored_type make(Tag tag, const Params& params,
    const A& a)
  {
    typedef typename is_given<Tag>::type is_given;
    return make_helper(tag, params, a, is_given());
  }

  template<typename Tag, typename A, typename B>
  static typename param<Tag>::stored_type make(Tag tag, const Params& params,
    const A& a, const B& b)
  {
    typedef typename is_given<Tag>::type is_given;
    return make_helper(tag, params, a, b, is_given());
  }

  template<typename Tag, typename A, typename B, typename C>
  static typename param<Tag>::stored_type make(Tag tag, const Params& params,
    const A& a, const B& b, const C& c)
  {
    typedef typename is_given<Tag>::type is_given;
    return make_helper(tag, params, a, b, c, is_given());
  }

  template<typename Tag, typename A, typename B, typename C
    , typename D>
  static typename param<Tag>::stored_type make(Tag tag, const Params& params,
    const A& a, const B& b, const C& c, const D& d)
  {
    typedef typename is_given<Tag>::type is_given;
    return make_helper(tag, params, a, b, c, d, is_given());
  }
}; 
}//namespace blink  

#endif // BLINK_DIJKSTRA_PARAMETER_HELPER_HPP