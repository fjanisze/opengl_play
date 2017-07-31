#include <new>
#include <cstdint>
#include <logger/logger.hpp>
#include <types.hpp>

#ifndef FACTORY_HPP
#define FACTORY_HPP

template< typename T >
struct factory {
    template< typename...Args >
    static typename T::pointer create( Args&& ...args )
    try
    {
        return std::make_shared< T >( std::forward<Args>( args )... );
    } catch ( std::exception& ex )
    {
        PANIC( "Failed while creating a new object, message: ",
               ex.what() );
        throw;
    }
};

namespace constants {

constexpr uint64_t INVALID_ID{ 0 };

}

template< typename T >
struct ids {
    static types::id_type create()
    {
        return next_id++;
    }

private:
    static types::id_type next_id;
};

template< typename T >
types::id_type ids< T >::next_id{ constants::INVALID_ID + 1 };

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
    operator types::id_type() const
    {
        return id;
    }
    const types::id_type id;
};

#endif
