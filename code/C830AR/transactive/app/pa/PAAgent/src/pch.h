#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4018 4146 4244 4250 4503 4786 4800 4819 4290 4996)
#endif

#if TA_PP_USE_PCH
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <boost/type_index.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>
#include <boost/hof.hpp>
#include <boost/hof/result.hpp>
#include <boost/predef.h>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
// #include <boost/thread/future.hpp>
#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/algorithm/cxx11/one_of.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>
//#include <boost/thread/futures/wait_for_all.hpp>
//#include <boost/thread/futures/wait_for_any.hpp>
//#include <boost/thread/future.hpp>
//#include <boost/thread/executors/basic_thread_pool.hpp>
//#include <boost/thread/executors/thread_executor.hpp>
//#include <boost/thread/executors/inline_executor.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/algorithm/string_regex.hpp>
// #include <boost/core/demangle.hpp>
// #include <boost/regex.hpp>
#include <boost/operators.hpp>

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <chrono>
#include <bitset>
#include <condition_variable>
#include <cstdint>
#include <cstddef>
#include <climits>
#endif
