
#include "gtest/gtest.h"

#include "System/Network.h"
#include "Network/NetworkMessage.h"

TEST(Network, SingleSerialize)
{
    const network::Address test_address = network::MakeAddress("127.0.0.1", 99);

    game::PingMessage ping_message;
    ping_message.header.id = 7;
    ping_message.header.sender = test_address;
    ping_message.local_time_stamp = 42;

    const std::vector<byte>& message_bytes = game::SerializeMessage(ping_message);
    const std::vector<byte_view>& message_views = game::UnpackMessageBuffer(message_bytes);

    EXPECT_EQ(1u, message_views.size());

    game::PingMessage deserialized_ping_message;
    EXPECT_TRUE(game::DeserializeMessage(message_views[0], deserialized_ping_message));

    EXPECT_EQ(7u, deserialized_ping_message.header.id);
    EXPECT_EQ(42u, deserialized_ping_message.local_time_stamp);
    EXPECT_EQ(test_address.host, deserialized_ping_message.header.sender.host);
    EXPECT_EQ(test_address.port, deserialized_ping_message.header.sender.port);
}

TEST(Network, BatchSerialize)
{
    game::PingMessage ping_message1;
    ping_message1.header.id = 1;
    ping_message1.local_time_stamp = 777;

    game::PingMessage ping_message2;
    ping_message2.header.id = 2;
    ping_message2.local_time_stamp = 666;

    game::TextMessage text_message1;
    text_message1.header.id = 3;
    std::sprintf(text_message1.text, "hello world!");

    game::TextMessage text_message2;
    text_message2.header.id = 4;
    std::sprintf(text_message2.text, "Goodbye cruel world!");
 
    std::vector<byte> message_buffer;
    game::PrepareMessageBuffer(message_buffer);

    EXPECT_TRUE(game::SerializeMessageToBuffer(ping_message1, message_buffer));
    EXPECT_TRUE(game::SerializeMessageToBuffer(ping_message2, message_buffer));
    EXPECT_TRUE(game::SerializeMessageToBuffer(text_message1, message_buffer));
    EXPECT_TRUE(game::SerializeMessageToBuffer(text_message2, message_buffer));

    uint32_t batch_id = -1;
    uint32_t batch_n_messages = 0;
    game::GetMessageBufferHeader(message_buffer, batch_id, batch_n_messages);

    EXPECT_EQ(0u, batch_id);
    EXPECT_EQ(4u, batch_n_messages);

    const std::vector<byte_view> message_views = game::UnpackMessageBuffer(message_buffer);
    EXPECT_EQ(4u, message_views.size());

    EXPECT_EQ(game::PingMessage::message_type, game::PeekMessageType(message_views[0]));
    EXPECT_EQ(game::PingMessage::message_type, game::PeekMessageType(message_views[1]));
    EXPECT_EQ(game::TextMessage::message_type, game::PeekMessageType(message_views[2]));
    EXPECT_EQ(game::TextMessage::message_type, game::PeekMessageType(message_views[3]));

    game::PingMessage deserialized_ping1;
    game::PingMessage deserialized_ping2;
    game::TextMessage deserialized_text1;
    game::TextMessage deserialized_text2;

    EXPECT_TRUE(game::DeserializeMessage(message_views[0], deserialized_ping1));
    EXPECT_TRUE(game::DeserializeMessage(message_views[1], deserialized_ping2));
    EXPECT_TRUE(game::DeserializeMessage(message_views[2], deserialized_text1));
    EXPECT_TRUE(game::DeserializeMessage(message_views[3], deserialized_text2));

    EXPECT_EQ(1u, deserialized_ping1.header.id);
    EXPECT_EQ(777u, deserialized_ping1.local_time_stamp);

    EXPECT_EQ(2u, deserialized_ping2.header.id);
    EXPECT_EQ(666u, deserialized_ping2.local_time_stamp);

    EXPECT_EQ(3u, deserialized_text1.header.id);
    EXPECT_STREQ("hello world!", deserialized_text1.text);

    EXPECT_EQ(4u, deserialized_text2.header.id);
    EXPECT_STREQ("Goodbye cruel world!", deserialized_text2.text);
}

TEST(Network, SerializeText)
{
    const network::Address test_address = network::MakeAddress("127.0.0.1", 99);

    std::vector<byte> message_buffer;
    game::PrepareMessageBuffer(message_buffer);

    game::TextMessage text_message1;
    text_message1.header.id = 1;
    text_message1.header.sender = test_address;
    std::sprintf(text_message1.text, "hello world!");

    game::TextMessage text_message2;
    text_message2.header.id = 2;
    text_message2.header.sender = test_address;
    std::sprintf(text_message2.text, "Goodbye cansas!");

    EXPECT_TRUE(game::SerializeMessageToBuffer(text_message1, message_buffer));
    EXPECT_TRUE(game::SerializeMessageToBuffer(text_message2, message_buffer));

    const std::vector<byte_view> message_views = game::UnpackMessageBuffer(message_buffer);
    EXPECT_EQ(2u, message_views.size());

    game::TextMessage deserialized_text1;
    game::TextMessage deserialized_text2;

    EXPECT_TRUE(game::DeserializeMessage(message_views[0], deserialized_text1));
    EXPECT_TRUE(game::DeserializeMessage(message_views[1], deserialized_text2));

    EXPECT_EQ(1u, deserialized_text1.header.id);
    EXPECT_EQ(test_address.host, deserialized_text1.header.sender.host);
    EXPECT_EQ(test_address.port, deserialized_text1.header.sender.port);
    EXPECT_STREQ("hello world!", deserialized_text1.text);

    EXPECT_EQ(2u, deserialized_text2.header.id);
    EXPECT_EQ(test_address.host, deserialized_text2.header.sender.host);
    EXPECT_EQ(test_address.port, deserialized_text2.header.sender.port);
    EXPECT_STREQ("Goodbye cansas!", deserialized_text2.text);
}
