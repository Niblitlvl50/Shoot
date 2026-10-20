
// No engine headers in here, windows.h (from httplib) clashes with them.
#include "HttpListener.h"

#include <thread>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "cpp-httplib/httplib.h"

struct editor::HttpListener::Impl
{
    httplib::Server server;
    std::thread server_thread;
    bool listening = false;
};

editor::HttpListener::HttpListener(const char* host, uint16_t port, HttpHandler handler)
    : m_impl(std::make_unique<Impl>())
{
    const auto http_handler = [handler](const httplib::Request& http_request, httplib::Response& http_response) {
        const HttpRequest request = { http_request.method, http_request.path, http_request.body };
        const HttpResponse response = handler(request);

        http_response.status = response.status;
        http_response.set_content(response.body, "application/json");
    };

    m_impl->server.Get(".*", http_handler);
    m_impl->server.Post(".*", http_handler);
    m_impl->server.Patch(".*", http_handler);
    m_impl->server.Delete(".*", http_handler);

    m_impl->listening = m_impl->server.bind_to_port(host, port);
    if(!m_impl->listening)
        return;

    Impl* impl = m_impl.get();
    m_impl->server_thread = std::thread([impl]() {
        impl->server.listen_after_bind();
    });
}

editor::HttpListener::~HttpListener()
{
    m_impl->server.stop();
    if(m_impl->server_thread.joinable())
        m_impl->server_thread.join();
}

bool editor::HttpListener::IsListening() const
{
    return m_impl->listening;
}
