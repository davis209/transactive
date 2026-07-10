// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#pragma warning (disable: 4819)

// #define BOOST_CHRONO_HEADER_ONLY
//#define BOOST_THREAD_PROVIDES_FUTURE
//#define BOOST_THREAD_PROVIDES_EXECUTORS
//#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assign.hpp>
//#include <boost/regex.hpp>

#include <boost/filesystem.hpp>

//#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/foreach.hpp>
//#include <boost/assign.hpp>
//#include <boost/preprocessor.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/process.hpp>

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/executors/scheduler.hpp>
#include <boost/thread/executors/thread_executor.hpp>
#include <boost/thread/executors/basic_thread_pool.hpp>
//#include <boost/thread/executors/inline_executor.hpp>
//#include <boost/thread/latch.hpp>
//#include <boost/thread/barrier.hpp>

//#include <boost/any.hpp>
//#include <boost/utility.hpp>
//#include <boost/type_traits.hpp>
#include <boost/format.hpp>
#include <boost/range.hpp>
#include <boost/range/join.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/bind/apply.hpp>
#include <boost/signals2.hpp>
#include <boost/utility.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/tti/tti.hpp>
#include <boost/tti/has_member_function.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/hana.hpp>
#include <boost/foreach.hpp>

//#include <boost/function_types/result_type.hpp>
//#include <boost/signals2.hpp>
//#include <boost/tuple/tuple.hpp>
//#include <boost/tuple/tuple_comparison.hpp>
//#include <boost/tuple/tuple_io.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/progress.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
