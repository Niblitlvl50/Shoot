
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace editor
{
    struct HttpRequest
    {
        std::string method;
        std::string path;
        std::string body;
    };

    struct HttpResponse
    {
        int status = 200;
        std::string body;
    };

    using HttpHandler = std::function<HttpResponse (const HttpRequest& request)>;

    // Wraps cpp-httplib, which has to live in a translation unit of its own since windows.h clashes
    // with engine names like mono::VariantTypeIndex::BOOL. Listens on its own thread and calls the handler
    // from there, for every GET, POST, PATCH and DELETE request.
    class HttpListener
    {
    public:

        HttpListener(const char* host, uint16_t port, HttpHandler handler);
        ~HttpListener();

        bool IsListening() const;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
