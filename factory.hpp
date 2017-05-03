#include <new>
#include <cstdint>
#include <logger/logger.hpp>

#ifndef FACTORY_HPP
#define FACTORY_HPP

template< typename T >
struct factory
{
    template< typename...Args >
    static typename T::pointer create( Args&&...args )
    try {
        return std::make_shared< T >( std::forward<Args>( args )... );
    } catch( std::exception& ex ) {
        PANIC("Failed while creating a new object, message: ",
              ex.what());
        throw;
    }
};

namespace constants
{

constexpr uint64_t INVALID_ID{ 0 };

}

template< typename T >
struct ids
{
    static uint64_t create() {
        return next_id++;
    }

private:
    static uint64_t next_id;
};

template< typename T >
uint64_t ids< T >::next_id{ constants::INVALID_ID + 1 };

/*
 * A class that generate unique IDs'
 * for the type T
 */
template< typename T >
class id_factory
{
public:
    id_factory() :
        id{ ids< T >::create() }
    { }
    operator uint64_t() const {
        return id;
    }
    const uint64_t id;
};

#endif
