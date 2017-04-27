#include <new>
#include <cstdint>
#include <logger/logger.hpp>

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
uint64_t ids< T >::next_id{ 1 };
