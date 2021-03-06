#ifndef _libslic3r_h_
#define _libslic3r_h_

// this needs to be included early for MSVC (listing it in Build.PL is not enough)
#include <ostream>
#include <iostream>
#include <queue>
#include <sstream>
#include <vector>
#include <boost/thread.hpp>

#define SLIC3R_VERSION "1.3.0-dev"

//FIXME This epsilon value is used for many non-related purposes:
// For a threshold of a squared Euclidean distance,
// for a trheshold in a difference of radians,
// for a threshold of a cross product of two non-normalized vectors etc.
#define EPSILON 1e-4
// Scaling factor for a conversion from coord_t to coordf_t: 10e-6
// This scaling generates a following fixed point representation with for a 32bit integer:
// 0..4294mm with 1nm resolution
#define SCALING_FACTOR 0.000001
// RESOLUTION, SCALED_RESOLUTION: Used as an error threshold for a Douglas-Peucker polyline simplification algorithm.
#define RESOLUTION 0.0125
#define SCALED_RESOLUTION (RESOLUTION / SCALING_FACTOR)
#define PI 3.141592653589793238
// When extruding a closed loop, the loop is interrupted and shortened a bit to reduce the seam.
#define LOOP_CLIPPING_LENGTH_OVER_NOZZLE_DIAMETER 0.15
// Maximum perimeter length for the loop to apply the small perimeter speed. 
#define SMALL_PERIMETER_LENGTH (6.5 / SCALING_FACTOR) * 2 * PI
#define INSET_OVERLAP_TOLERANCE 0.4
#define EXTERNAL_INFILL_MARGIN 3
#define scale_(val) ((val) / SCALING_FACTOR)
#define unscale(val) ((val) * SCALING_FACTOR)
#define SCALED_EPSILON scale_(EPSILON)
typedef long coord_t;
typedef double coordf_t;

/* Implementation of CONFESS("foo"): */
#ifdef _MSC_VER
	#define CONFESS(...) confess_at(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
	#define CONFESS(...) confess_at(__FILE__, __LINE__, __func__, __VA_ARGS__)
#endif
void confess_at(const char *file, int line, const char *func, const char *pat, ...);
/* End implementation of CONFESS("foo"): */

// Which C++ version is supported?
// For example, could optimized functions with move semantics be used?
#if __cplusplus==201402L
	#define SLIC3R_CPPVER 14
	#define STDMOVE(WHAT) std::move(WHAT)
#elif __cplusplus==201103L
	#define SLIC3R_CPPVER 11
	#define STDMOVE(WHAT) std::move(WHAT)
#else
	#define SLIC3R_CPPVER 0
	#define STDMOVE(WHAT) (WHAT)
#endif

namespace Slic3r {

enum Axis { X=0, Y, Z };

template <class T>
inline void append_to(std::vector<T> &dst, const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

template <class T> void
_parallelize_do(std::queue<T>* queue, boost::mutex* queue_mutex, boost::function<void(T)> func)
{
    //std::cout << "THREAD STARTED: " << boost::this_thread::get_id() << std::endl;
    while (true) {
        T i;
        {
            boost::lock_guard<boost::mutex> l(*queue_mutex);
            if (queue->empty()) return;
            i = queue->front();
            queue->pop();
        }
        //std::cout << "  Thread " << boost::this_thread::get_id() << " processing item " << i << std::endl;
        func(i);
        boost::this_thread::interruption_point();
    }
}

template <class T> void
parallelize(std::queue<T> queue, boost::function<void(T)> func,
    int threads_count = boost::thread::hardware_concurrency())
{
    boost::mutex queue_mutex;
    boost::thread_group workers;
    for (int i = 0; i < threads_count; i++)
        workers.add_thread(new boost::thread(&_parallelize_do<T>, &queue, &queue_mutex, func));
    workers.join_all();
}

template <class T> void
parallelize(T start, T end, boost::function<void(T)> func,
    int threads_count = boost::thread::hardware_concurrency())
{
    std::queue<T> queue;
    for (T i = start; i <= end; ++i) queue.push(i);
    parallelize(queue, func, threads_count);
}

} // namespace Slic3r

using namespace Slic3r;

#endif
