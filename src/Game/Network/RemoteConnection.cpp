
#include "RemoteConnection.h"
#include "MessageDispatcher.h"

#include <algorithm>

#include "huffandpuff/huffman.h"

using namespace game;

namespace
{
    void ReceiveFunc(
        network::ISocket* socket, MessageDispatcher* dispatcher, ConnectionStats& connection_stats, bool& stop)
    {
        unsigned char huffbuf_heap[HUFFHEAP_SIZE];
        std::vector<byte> message_buffer(NetworkMessageBufferSize, '\0');

        NetworkMessage message;
        message.payload.resize(NetworkMessageBufferSize);

        while(!stop)
        {
            const int bytes_received = socket->Receive(message_buffer, &message.address);
            if(bytes_received > 0)
            {
                std::fill(message.payload.begin(), message.payload.begin() + message.payload.size(), '\0');

                const unsigned long decompressed_size = huffman_decompress(
                    message_buffer.data(), bytes_received, message.payload.data(), message.payload.size(), huffbuf_heap);

                assert(decompressed_size != 0);

                connection_stats.total_packages_received++;
                connection_stats.total_byte_received += decompressed_size;
                connection_stats.total_compressed_byte_received += bytes_received;

                dispatcher->PushNewMessage(message);
            }
            else
            {
                std::this_thread::yield();
            }
        }
    };

    void SendFunc(
        network::ISocket* socket, RemoteConnection::OutgoingMessages* out_messages, ConnectionStats& connection_stats, bool& stop)
    {
        unsigned char huffbuf_heap[HUFFHEAP_SIZE];

        std::vector<byte> compressed_bytes;
        compressed_bytes.resize(NetworkMessageBufferSize, '\0');

        while(!stop)
        {
            bool no_messages = true;

            {
                std::lock_guard<std::mutex> lock(out_messages->message_mutex);
                for(const NetworkMessage& message : out_messages->unhandled_messages)
                {
                    const unsigned long compressed_size = huffman_compress(
                        message.payload.data(), message.payload.size(), compressed_bytes.data(), compressed_bytes.size(), huffbuf_heap);

                    if(compressed_size == 0)
                        std::printf("RemoteConnection|Failed to compress message.\n");
                    else if(compressed_size > message.payload.size())
                        std::printf("RemoteConnection|Compressed size is more than uncompressed!!!\n");
                    else
                    {
                        if(socket->Send(compressed_bytes.data(), compressed_size, message.address))
                        {
                            connection_stats.total_packages_sent++;
                            connection_stats.total_byte_sent += message.payload.size();
                            connection_stats.total_compressed_byte_sent += compressed_size;
                        }
                    }
                }

                no_messages = out_messages->unhandled_messages.empty();
                out_messages->unhandled_messages.clear();
            }

            if(no_messages)
                std::this_thread::yield();
        }
    }
}

RemoteConnection::RemoteConnection(MessageDispatcher* dispatcher, network::ISocketPtr socket)
    : m_stop(false)
    , m_socket(std::move(socket))
{
    m_stats = { 0, 0, 0, 0, 0, 0 };
    m_receive_thread = std::thread(ReceiveFunc, m_socket.get(), dispatcher, std::ref(m_stats), std::ref(m_stop));
    m_send_thread = std::thread(SendFunc, m_socket.get(), &m_messages, std::ref(m_stats), std::ref(m_stop));
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_receive_thread.join();
    m_send_thread.join();
}

void RemoteConnection::SendMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_messages.message_mutex);
    m_messages.unhandled_messages.push_back(message);
}

const ConnectionStats& RemoteConnection::GetConnectionStats() const
{
    return m_stats;
}
