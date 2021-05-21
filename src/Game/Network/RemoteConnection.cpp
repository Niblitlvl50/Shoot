
#include "RemoteConnection.h"
#include "MessageDispatcher.h"
#include "NetworkSerialize.h"
#include "System/System.h"

#include <algorithm>
#include <cassert>

#include "huffandpuff/huffman.h"

using namespace game;

namespace
{
    void ReceiveFunc(
        network::ISocket* socket, MessageDispatcher* dispatcher, ConnectionStats& connection_stats, bool& stop)
    {
        unsigned char huffbuf_heap[HUFFHEAP_SIZE];
        std::vector<byte> message_buffer(NetworkMessageBufferTotalSize, '\0');

        NetworkMessage message;
        message.payload.resize(NetworkMessageBufferTotalSize);

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
        }
    };

    void SendFunc(
        network::ISocket* socket, RemoteConnection::OutgoingMessages* out_messages, ConnectionStats& connection_stats, bool& stop)
    {
        unsigned char huffbuf_heap[HUFFHEAP_SIZE];

        std::vector<byte> compressed_bytes;
        compressed_bytes.resize(NetworkMessageBufferTotalSize, '\0');

        while(!stop)
        {
            std::unique_lock<std::mutex> lock(out_messages->message_mutex);
            out_messages->message_signal.wait(lock);

            for(const RemoteConnection::Message& message : out_messages->unhandled_messages)
            {
                const unsigned long compressed_size = huffman_compress(
                    message.payload.data(), message.payload.size(), compressed_bytes.data(), compressed_bytes.size(), huffbuf_heap);

                if(compressed_size == 0)
                {
                    System::Log("RemoteConnection|Failed to compress message.");
                    continue;
                }

                if(compressed_size > message.payload.size())
                {
                    /*
                    const float compression_ratio = float(compressed_size) / float(message.payload.size());
                    System::Log(
                        "RemoteConnection|Warning, compressed size(%lu) is more than uncompressed(%lu)!!! ratio: %f, diff: %lu",
                        compressed_size,
                        message.payload.size(), 
                        compression_ratio,
                        compressed_size - message.payload.size());
                        */
                }

                for(const network::Address& address : message.addresses)
                {
                    if(socket->Send(compressed_bytes.data(), compressed_size, address))
                    {
                        connection_stats.total_packages_sent++;
                        connection_stats.total_byte_sent += message.payload.size();
                        connection_stats.total_compressed_byte_sent += compressed_size;
                    }
                }
            }

            out_messages->unhandled_messages.clear();
        }
    }
}

RemoteConnection::RemoteConnection(MessageDispatcher* dispatcher, network::ISocketPtr socket)
    : m_stop(false)
    , m_socket(std::move(socket))
    , m_sequence_id(0)
{
    m_stats = { 0, 0, 0, 0, 0, 0 };
    m_receive_thread = std::thread(ReceiveFunc, m_socket.get(), dispatcher, std::ref(m_stats), std::ref(m_stop));
    m_send_thread = std::thread(SendFunc, m_socket.get(), &m_messages, std::ref(m_stats), std::ref(m_stop));
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_messages.message_signal.notify_one();

    m_receive_thread.join();
    m_send_thread.join();
}

void RemoteConnection::SendData(const std::vector<byte>& data, const network::Address& target)
{
    const std::vector<network::Address> addresses = { target };
    SendData(data, addresses);
}

void RemoteConnection::SendData(const std::vector<byte>& data, const std::vector<network::Address>& addresses)
{
    {
        std::lock_guard<std::mutex> lock(m_messages.message_mutex);

        ++m_sequence_id;

        std::vector<byte>& mutable_data = const_cast<std::vector<byte>&>(data);
        NetworkMessageHeader header = GetMessageBufferHeader(mutable_data);
        header.id = m_sequence_id;
        SetMessageBufferHeader(mutable_data, header);

        m_messages.unhandled_messages.push_back({ data, addresses });
    }

    m_messages.message_signal.notify_one();
}

const ConnectionStats& RemoteConnection::GetConnectionStats() const
{
    return m_stats;
}
