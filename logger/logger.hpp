#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "log.hpp"

extern logging::logger< logging::file_log_policy > log_inst;

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define CURRENT_FUNCTION_NAME __FUNCTION__ //__PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define CURRENT_FUNCTION_NAME __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define CURRENT_FUNCTION_NAME __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define CURRENT_FUNCTION_NAME __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define CURRENT_FUNCTION_NAME __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define CURRENT_FUNCTION_NAME __func__
#else
#define CURRENT_FUNCTION_NAME "(unknown)"
#endif

#define LOG1(...) log_inst.print< logging::severity_type::debug1 >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define LOG2(...) log_inst.print< logging::severity_type::debug2 >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define LOG3(...) log_inst.print< logging::severity_type::debug3 >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define ERR(...) log_inst.print< logging::severity_type::error >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define WARN1(...) log_inst.print< logging::severity_type::warning1 >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define WARN2(...) log_inst.print< logging::severity_type::warning2 >(CURRENT_FUNCTION_NAME,": ",__VA_ARGS__)
#define PANIC(...) \
    do{ \
        ERR(__VA_ARGS__); \
        throw std::runtime_error(std::string("Unrecoverable error in ")+CURRENT_FUNCTION_NAME+std::string(",check the logs!")); \
    }while(0)
#define DUMP_VEC3(msg,vec) LOG1(msg," x:",vec.x,",y:",vec.y,",z:",vec.z)

#define SET_LOG_THREAD_NAME(name) log_inst.set_thread_name(name);

#endif
