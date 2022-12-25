#ifndef LIBIM_BINARYSTREAM_H
#define LIBIM_BINARYSTREAM_H
#include "stream.h"

#include <cstdint>
#include <type_traits>

namespace libim {

    template<typename T, typename Iterator = typename T::iterator>
    class BinaryStream : public virtual Stream
    {
    public:
        BinaryStream(T& data);
        BinaryStream(T& data, std::size_t size);
        BinaryStream(T& data, Iterator first, Iterator last);
        virtual ~BinaryStream() override;

        virtual void seek(std::size_t position) const override;
        std::size_t capacity() const;
        void reserve(std::size_t);
        virtual std::size_t size() const override;
        virtual std::size_t tell() const override;
        virtual bool canRead() const override;
        virtual bool canWrite() const override;

    protected:
        virtual void flush() override {}
        virtual std::size_t readsome(byte_t* data, std::size_t length) const override;
        virtual std::size_t writesome(const byte_t* data, std::size_t length) override;

    private:
        template<typename MemFp>
        void resize(std::size_t size, MemFp&& func);

    private:
        T& data_;
        mutable Iterator begin_;
        mutable Iterator end_;
        mutable Iterator pos_;
    };

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

    template<typename T, typename ConstIterator = decltype(std::cbegin(std::declval<T>()))>
    class InputBinaryStream final : public InputStream, public BinaryStream<const T, ConstIterator>
    {
    public:
        InputBinaryStream(const T& data) :
            BinaryStream<const T, ConstIterator>(data)
        {}

        InputBinaryStream(const T& data, std::size_t size) :
            BinaryStream<const T, ConstIterator>(data, size)
        {}

        InputBinaryStream(const T& data, ConstIterator first, ConstIterator last) :
            BinaryStream<const T, ConstIterator>(data, first, last)
        {}

    private:
        using BinaryStream<const T, ConstIterator>::write;
    };

    template<typename T, typename Iterator = typename T::iterator>
    class OutputBinaryStream final : public OutputStream, public BinaryStream<T, Iterator>
    {
    public:
        OutputBinaryStream(T& data) : BinaryStream<T, Iterator>(data)
        {
            static_assert(!std::is_const_v<T>, "T must not be const-qualified type");
        }

        OutputBinaryStream(T& data, std::size_t size) : BinaryStream<T, Iterator>(data, size)
        {
            static_assert(!std::is_const_v<T>, "T must not be const-qualified type");
        }

        OutputBinaryStream(T& data, Iterator first, Iterator last) :
            BinaryStream<T, Iterator>(data, first, last)
        {
            static_assert(!std::is_const_v<T>, "T must not be const-qualified type");
        }

    private:
        using BinaryStream<T, Iterator>::read;
    };

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

#include "impl/binarystream.impl"
#endif // LIBIM_BINARYSTREAM_H
