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

    template<typename T>
    class InputBinaryStream final : public InputStream, public BinaryStream<const T, typename T::const_iterator>
    {
    public:
        InputBinaryStream(const T& data) :
            BinaryStream<const T, typename T::const_iterator>(data)
        {}

        InputBinaryStream(const T& data, typename T::const_iterator first, typename T::const_iterator last) :
            BinaryStream<const T, typename T::const_iterator>(data, first, last)
        {}

    private:
        using BinaryStream<const T, typename T::const_iterator>::write;
    };

    template<typename T>
    class OutputBinaryStream final : public OutputStream, public BinaryStream<T, typename T::iterator>
    {
    public:
        OutputBinaryStream(T& data) :
            BinaryStream<T>(data)
        {
            static_assert(!std::is_const_v<T>, "T must not be const-qualified type");
        }

        OutputBinaryStream(T& data, typename T::iterator first, typename T::iterator last) :
            BinaryStream<T, typename T::iterator>(data, first, last)
        {
            static_assert(!std::is_const_v<T>, "T must not be const-qualified type");
        }

    private:
        using BinaryStream<T, typename T::iterator>::read;
    };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}

#include "impl/binarystream.impl"
#endif // LIBIM_BINARYSTREAM_H
