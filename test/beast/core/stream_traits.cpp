//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

// Test that header file is self-contained.
#include <boost/beast/core/stream_traits.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/asio/io_context.hpp>

namespace boost {
namespace beast {

class stream_traits_test
    : public beast::unit_test::suite
{
public:
    struct without
    {
        int dummy = 0;

        without() = default;

        template<class T>
        std::size_t write_some(T const&)
        {
            return 0;
        }

        template<class T>
        std::size_t write_some(T const&, boost::system::error_code&)
        {
            return 0;
        }
    };

    template<class T>
    struct with
    {
        T t;

        with() = default;

        T&
        next_layer()
        {
            return t;
        }

        T const&
        next_layer() const
        {
            return t;
        }
    };

    BOOST_STATIC_ASSERT(
        ! detail::has_next_layer<without>::value);

    BOOST_STATIC_ASSERT(
        detail::has_next_layer<with<without>>::value);

    BOOST_STATIC_ASSERT(
        detail::has_next_layer<with<with<without>>>::value);

    void
    testGetLowestLayer()
    {
        {
            without w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w);
        }
        {
            without const w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w);
        }
        {
            with<without> w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t);
        }
        {
            with<without> const w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t);
        }
        {
            with<with<without>> w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t.t);
        }
        {
            with<with<without>> const w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t.t);
        }
        {
            with<with<with<without>>> w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t.t.t);
        }
        {
            with<with<with<without>>> const w{};
            BEAST_EXPECT(&get_lowest_layer(w) == &w.t.t.t);
        }
    }

    //--------------------------------------------------------------------------
/*
    @par Example
    This code implements a <em>SyncWriteStream</em> wrapper which calls
    `std::terminate` upon any error.
*/
    template <class NextLayer>
    class write_stream
    {
        NextLayer next_layer_;

    public:
        static_assert(is_sync_write_stream<NextLayer>::value,
            "SyncWriteStream requirements not met");

        template<class... Args>
        explicit
        write_stream(Args&&... args)
            : next_layer_(std::forward<Args>(args)...)
        {
        }

        NextLayer& next_layer() noexcept
        {
            return next_layer_;
        }

        NextLayer const& next_layer() const noexcept
        {
            return next_layer_;
        }

        template<class ConstBufferSequence>
        std::size_t
        write_some(ConstBufferSequence const& buffers)
        {
            error_code ec;
            auto const bytes_transferred = next_layer_.write_some(buffers, ec);
            if(ec)
                std::terminate();
            return bytes_transferred;
        }

        template<class ConstBufferSequence>
        std::size_t
        write_some(ConstBufferSequence const& buffers, error_code& ec)
        {
            auto const bytes_transferred = next_layer_.write_some(buffers, ec);
            if(ec)
                std::terminate();
            return bytes_transferred;
        }
    };
    
    void
    testGetLowestLayerJavadoc()
    {
        write_stream<without> s;
        BOOST_STATIC_ASSERT(
            is_sync_write_stream<without>::value);
        BOOST_STATIC_ASSERT(std::is_same<
            decltype(get_lowest_layer(s)), without&>::value);

#if 0
        BEAST_EXPECT(static_cast<
            std::size_t(type::*)(net::const_buffer)>(
                &type::write_some<net::const_buffer>));
#endif
    }

    //--------------------------------------------------------------------------

    void
    testExecutorType()
    {
    }

    void
    testExecutorTypeJavadoc()
    {
    }

    //--------------------------------------------------------------------------

    struct sync_read_stream
    {
        template<class MutableBufferSequence>
        std::size_t
        read_some(MutableBufferSequence const&);

        template<class MutableBufferSequence>
        std::size_t
        read_some(MutableBufferSequence const&,
            error_code& ec);
    };

    struct sync_write_stream
    {
        template<class ConstBufferSequence>
        std::size_t
        write_some(ConstBufferSequence const&);

        template<class ConstBufferSequence>
        std::size_t
        write_some(
            ConstBufferSequence const&, error_code&);
    };

    struct async_read_stream
    {
        net::io_context::executor_type
        get_executor();

        template<class MutableBufferSequence, class ReadHandler>
        void
        async_read_some(
            MutableBufferSequence const&, ReadHandler&&);
    };

    struct async_write_stream
    {
        net::io_context::executor_type
        get_executor();

        template<class ConstBufferSequence, class WriteHandler>
        void
        async_write_some(
            ConstBufferSequence const&, WriteHandler&&);
    };

    struct sync_stream : sync_read_stream, sync_write_stream
    {
    };

    struct async_stream : async_read_stream, async_write_stream
    {
        using async_read_stream::get_executor;
    };

    BOOST_STATIC_ASSERT(is_sync_read_stream<sync_read_stream>::value);
    BOOST_STATIC_ASSERT(is_sync_write_stream<sync_write_stream>::value);
    BOOST_STATIC_ASSERT(is_sync_read_stream<sync_stream>::value);
    BOOST_STATIC_ASSERT(is_sync_write_stream<sync_stream>::value);
    BOOST_STATIC_ASSERT(is_sync_stream<sync_stream>::value);

    BOOST_STATIC_ASSERT(is_async_read_stream<async_read_stream>::value);
    BOOST_STATIC_ASSERT(is_async_write_stream<async_write_stream>::value);
    BOOST_STATIC_ASSERT(is_async_read_stream<async_stream>::value);
    BOOST_STATIC_ASSERT(is_async_write_stream<async_stream>::value);
    BOOST_STATIC_ASSERT(is_async_stream<async_stream>::value);

    BOOST_STATIC_ASSERT(! is_sync_read_stream<sync_write_stream>::value);
    BOOST_STATIC_ASSERT(! is_sync_write_stream<sync_read_stream>::value);
    BOOST_STATIC_ASSERT(! is_async_read_stream<async_write_stream>::value);
    BOOST_STATIC_ASSERT(! is_async_write_stream<async_read_stream>::value);

    BOOST_STATIC_ASSERT(! is_sync_stream<async_stream>::value);
    BOOST_STATIC_ASSERT(! is_async_stream<sync_stream>::value);

    //--------------------------------------------------------------------------

    void
    run() override
    {
        testGetLowestLayer();
        testGetLowestLayerJavadoc();
        testExecutorType();
        testExecutorTypeJavadoc();
    }
};

BEAST_DEFINE_TESTSUITE(beast,core,stream_traits);

} // beast
} // boost
