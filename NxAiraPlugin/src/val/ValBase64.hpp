#ifndef _ValBase64_h
#define _ValBase64_h

#include <string>
#include <vector>
#include <string_view>
#include <stdexcept>
#include <cmath>

#include <boost/beast/core/span.hpp>
#include <libbase64.h>


class ValBase64 {
private:
    static constexpr size_t BASE64CORESIZE = 13;
    static std::invalid_argument invalid;

#pragma region "Decode Function"
private:
    inline static void decode_shared_impl(const std::string_view& input, bool getMimeType, std::string& mime, size_t& size, size_t& mimeSize, size_t& estimatelen) {
        size = input.size();
        
        /// replace mime
        const char* t = input.data();
        mimeSize = 0;
        if (t[0] == 'd' && t[1] == 'a' && t[2] == 't' && t[3] == 'a' && t[4] == ':') {
            if (size < BASE64CORESIZE) throw invalid;
            size_t j = 5;
            for (; j<size; ++j)
                if (t[j] == ';') break;
            if (j == size) throw invalid;
            mimeSize = j-5;
            if (getMimeType) mime = std::string(t+5, mimeSize);
            mimeSize += BASE64CORESIZE;
        }
        size -= mimeSize;

        estimatelen = std::ceil((float)size/4)*3;
    }

    template<typename T>
    inline static std::vector<T> decode_impl(const std::string_view& input, bool getMimeType, std::string& mime, std::vector<T>*) {
        // std::cout << "decode with std::vector" << std::endl;
        size_t size, mimeSize, estimatelen, outlen;
        decode_shared_impl(input, getMimeType, mime, size, mimeSize, estimatelen);
        auto data = input.data();
        std::vector<T> rtn(estimatelen);
        if (!base64_decode(data+mimeSize, size, reinterpret_cast<char*>(rtn.data()), &outlen, 0)) throw invalid;
        if (outlen != estimatelen) rtn.resize(outlen);
        return rtn;
    }
    template<typename T>
    inline static boost::beast::span<T> decode_impl(const std::string_view& input, bool getMimeType, std::string& mime, boost::beast::span<T>*) {
        // std::cout << "decode with boost::beast::span" << std::endl;
        size_t size, mimeSize, estimatelen, outlen;
        decode_shared_impl(input, getMimeType, mime, size, mimeSize, estimatelen);
        auto data = input.data();
        boost::beast::span<T> rtn(new T[estimatelen], estimatelen);
        if (!base64_decode(data+mimeSize, size, reinterpret_cast<char*>(rtn.data()), &outlen, 0)) throw invalid;
        if (outlen != estimatelen) rtn = boost::beast::span<T>(rtn.data(), outlen);
        return rtn;
    }

    inline static std::string decode_impl(const std::string_view& input, bool getMimeType, std::string& mime, std::string*) {
        // std::cout << "decode with std::string" << std::endl;
        size_t size, mimeSize, estimatelen, outlen;
        decode_shared_impl(input, getMimeType, mime, size, mimeSize, estimatelen);
        auto data = input.data();
        std::string rtn(0, ' ');
        // std::string rtn;
        rtn.resize(estimatelen);
        //std::string rtn(estimatelen, '\0');
        if (!base64_decode(data+mimeSize, size, reinterpret_cast<char*>(rtn.data()), &outlen, 0)) throw invalid;
        if (outlen != estimatelen) rtn.resize(outlen);
        return rtn;
    }

public:
    template<typename T = std::vector<char>>
    static decltype(auto) Decode(const std::string_view& input, bool getMimeType = false) {
        std::string mime;
        return Decode<T>(input, getMimeType, mime);
    }
    template<typename T = std::vector<char>>
    static decltype(auto) Decode(const std::string_view& input, bool getMimeType, std::string& mime) {
        return decode_impl(input, getMimeType, mime, static_cast<T*>(0));
    }
#pragma endregion "Decode Function"

#pragma region "Encode Function"
private:
    inline static void encode_shared_impl(const std::string_view& input, const std::string& mime, bool& withMime, size_t& size, size_t& mimeSize, size_t& estimatelen) {
        // auto src = input.data();
        size = input.size();
        size_t len = std::ceil((float)size/3)*4;
        mimeSize = mime.size();
        withMime = mimeSize > 0;
        if (withMime) mimeSize += BASE64CORESIZE;
        estimatelen = len + mimeSize;
    }
    template<typename T>
    inline static void encode_shared_doencode_impl(const std::string_view& input, const T* outputdata, const std::string& mime, bool withMime, size_t& size, size_t& mimeSize) {
        auto src = input.data();
        char* target = const_cast<char*>(outputdata);
        if (withMime) sprintf(target, "%s%s%s", "data:", mime.data(), ";base64,");
        size_t outlen;
        base64_encode(src, size, target + mimeSize, &outlen, 0);
    }

    template<typename T>
    inline static boost::beast::span<T> encode_impl(const std::string_view& input, const std::string& mime, boost::beast::span<T>*) {
        // std::cout << "encode with boost::beast::span" << std::endl;
        bool withMime;
        size_t size, mimeSize, estimatelen;
        encode_shared_impl(input, mime, withMime, size, mimeSize, estimatelen);
        boost::beast::span<T> rtn(new T[estimatelen], estimatelen);
        encode_shared_doencode_impl(input, rtn.data(), mime, withMime, size, mimeSize);
        return rtn;
    }

    template<typename T>
    inline static std::vector<T> encode_impl(const std::string_view& input, const std::string& mime, std::vector<T>*) {
        // std::cout << "encode with std::vector" << std::endl;
        bool withMime;
        size_t size, mimeSize, estimatelen;
        encode_shared_impl(input, mime, withMime, size, mimeSize, estimatelen);
        std::vector<T> rtn(estimatelen);
        encode_shared_doencode_impl(input, rtn.data(), mime, withMime, size, mimeSize);
        return rtn;
    }

    inline static std::string encode_impl(const std::string_view& input, const std::string& mime, std::string*) {
        // std::cout << "encode with std::string" << std::endl;
        bool withMime;
        size_t size, mimeSize, estimatelen;
        encode_shared_impl(input, mime, withMime, size, mimeSize, estimatelen);
        std::string rtn(0, ' ');
        rtn.resize(estimatelen);
        encode_shared_doencode_impl(input, rtn.data(), mime, withMime, size, mimeSize);
        return rtn;
    }

public:
    template<typename T = std::vector<char>>
    static decltype(auto) Encode(const std::string_view& input, const std::string& mime) {
        return encode_impl(input, mime, static_cast<T*>(0));
    }
    template<typename T = std::vector<char>, typename U>
    static decltype(auto) Encode(const std::vector<U>& input, const std::string& mime) {
        return encode_impl(std::string_view(reinterpret_cast<const char*>(input.data()), input.size()), mime, static_cast<T*>(0));
    }

#pragma endregion "Encode Function"

};

#endif