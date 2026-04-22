/* test_mqtt_packet.c
 *
 * Copyright (C) 2006-2026 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 *
 * wolfMQTT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfMQTT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

/* mqtt_client.h must be included before mqtt_packet.h: it pulls in
 * wolfmqtt/options.h (WOLFMQTT_V5 et al.) for non-autotools builds. */
#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt/mqtt_packet.h"
#include "tests/unit_test.h"

void run_mqtt_packet_tests(void);

static void setup(void)     { }
static void teardown(void)  { }

/* ============================================================================
 * MqttEncode_Num / MqttDecode_Num
 * ============================================================================ */

TEST(encode_num_basic)
{
    byte buf[2];
    int ret = MqttEncode_Num(buf, 0x1234);
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0x12, buf[0]);
    ASSERT_EQ(0x34, buf[1]);
}

TEST(encode_num_zero)
{
    byte buf[2];
    int ret = MqttEncode_Num(buf, 0);
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0, buf[0]);
    ASSERT_EQ(0, buf[1]);
}

TEST(encode_num_max)
{
    byte buf[2];
    int ret = MqttEncode_Num(buf, 0xFFFF);
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0xFF, buf[0]);
    ASSERT_EQ(0xFF, buf[1]);
}

TEST(encode_num_null_buf)
{
    /* NULL buf: length-only probe; returns size without writing */
    int ret = MqttEncode_Num(NULL, 0x1234);
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
}

TEST(decode_num_basic)
{
    byte buf[2] = { 0x12, 0x34 };
    word16 val = 0;
    int ret = MqttDecode_Num(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0x1234, val);
}

TEST(decode_num_zero)
{
    byte buf[2] = { 0x00, 0x00 };
    word16 val = 0xFFFF;
    int ret = MqttDecode_Num(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0, val);
}

TEST(decode_num_max)
{
    byte buf[2] = { 0xFF, 0xFF };
    word16 val = 0;
    int ret = MqttDecode_Num(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0xFFFF, val);
}

TEST(decode_num_buffer_too_small)
{
    byte buf[1] = { 0x12 };
    word16 val = 0;
    int ret = MqttDecode_Num(buf, &val, 1);
    ASSERT_EQ(MQTT_CODE_ERROR_OUT_OF_BUFFER, ret);
}

TEST(encode_decode_num_roundtrip)
{
    byte buf[2];
    word16 val;
    int ret;

    ret = MqttEncode_Num(buf, 0xABCD);
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ret = MqttDecode_Num(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_DATA_LEN_SIZE, ret);
    ASSERT_EQ(0xABCD, val);
}

/* ============================================================================
 * MqttEncode_Vbi / MqttDecode_Vbi
 * ============================================================================ */

TEST(encode_vbi_one_byte)
{
    byte buf[4];
    int len = MqttEncode_Vbi(buf, 127);
    ASSERT_EQ(1, len);
    ASSERT_EQ(0x7F, buf[0]);
}

TEST(encode_vbi_two_bytes)
{
    byte buf[4];
    int len = MqttEncode_Vbi(buf, 128);
    ASSERT_EQ(2, len);
}

TEST(encode_vbi_three_bytes)
{
    byte buf[4];
    int len = MqttEncode_Vbi(buf, 16384);
    ASSERT_EQ(3, len);
}

TEST(encode_vbi_four_bytes)
{
    byte buf[4];
    int len = MqttEncode_Vbi(buf, 2097152);
    ASSERT_EQ(4, len);
}

TEST(encode_vbi_null_buf)
{
    ASSERT_EQ(1, MqttEncode_Vbi(NULL, 127));
    ASSERT_EQ(2, MqttEncode_Vbi(NULL, 128));
    ASSERT_EQ(3, MqttEncode_Vbi(NULL, 16384));
    ASSERT_EQ(4, MqttEncode_Vbi(NULL, 2097152));
}

TEST(decode_vbi_one_byte_zero)
{
    byte buf[1] = { 0x00 };
    word32 val = 0xFFFFFFFF;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(1, rc);
    ASSERT_EQ(0, val);
}

TEST(decode_vbi_one_byte_max)
{
    byte buf[1] = { 0x7F };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(1, rc);
    ASSERT_EQ(127, val);
}

TEST(decode_vbi_two_bytes_min)
{
    byte buf[2] = { 0x80, 0x01 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(2, rc);
    ASSERT_EQ(128, val);
}

TEST(decode_vbi_two_bytes_max)
{
    byte buf[2] = { 0xFF, 0x7F };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(2, rc);
    ASSERT_EQ(16383, val);
}

TEST(decode_vbi_three_bytes_max)
{
    byte buf[3] = { 0xFF, 0xFF, 0x7F };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(3, rc);
    ASSERT_EQ(2097151, val);
}

TEST(decode_vbi_four_bytes_max)
{
    byte buf[4] = { 0xFF, 0xFF, 0xFF, 0x7F };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(4, rc);
    ASSERT_EQ(268435455, val);
}

TEST(decode_vbi_five_byte_malformed)
{
    byte buf[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_vbi_buffer_too_small)
{
    byte buf[1] = { 0x80 };   /* needs a continuation byte */
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, 1);
    ASSERT_EQ(MQTT_CODE_ERROR_OUT_OF_BUFFER, rc);
}

/* [MQTT-1.5.5-1] Overlong encodings must be rejected */
TEST(decode_vbi_overlong_2byte_zero)
{
    byte buf[2] = { 0x80, 0x00 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_vbi_overlong_3byte_zero)
{
    byte buf[3] = { 0x80, 0x80, 0x00 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_vbi_overlong_4byte_zero)
{
    byte buf[4] = { 0x80, 0x80, 0x80, 0x00 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_vbi_overlong_2byte_127)
{
    byte buf[2] = { 0xFF, 0x00 };
    word32 val = 0;
    int rc = MqttDecode_Vbi(buf, &val, sizeof(buf));
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(encode_decode_vbi_roundtrip)
{
    byte buf[4];
    word32 val;
    int enc, dec;

    /* Boundary values covering each VBI size */
    const word32 cases[] = { 0, 127, 128, 16383, 16384, 2097151, 2097152,
                             268435455 };
    size_t i;
    for (i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        enc = MqttEncode_Vbi(buf, cases[i]);
        ASSERT_TRUE(enc > 0);
        val = 0;
        dec = MqttDecode_Vbi(buf, &val, sizeof(buf));
        ASSERT_EQ(enc, dec);
        ASSERT_EQ(cases[i], val);
    }
}

/* ============================================================================
 * MqttEncode_Publish
 * ============================================================================ */

TEST(encode_publish_qos1_packet_id_zero)
{
    byte tx_buf[256];
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_1;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    ASSERT_EQ(MQTT_CODE_ERROR_PACKET_ID, rc);
}

TEST(encode_publish_qos2_packet_id_zero)
{
    byte tx_buf[256];
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_2;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    ASSERT_EQ(MQTT_CODE_ERROR_PACKET_ID, rc);
}

TEST(encode_publish_qos0_packet_id_zero_ok)
{
    byte tx_buf[256];
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_0;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    ASSERT_TRUE(rc > 0);
}

TEST(encode_publish_qos1_valid)
{
    byte tx_buf[256];
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_1;
    pub.packet_id = 1;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    ASSERT_TRUE(rc > 0);
}

/* ============================================================================
 * MqttDecode_Publish
 * ============================================================================ */

TEST(decode_publish_qos0_valid)
{
    /* Fixed header (PUBLISH, QoS 0, remain_len=7), topic "a/b",
     * payload "HI". Using nonzero payload bytes catches a
     * qos>MQTT_QOS_0 -> qos>=MQTT_QOS_0 mutation that would read
     * the first 2 payload bytes as a spurious packet_id. */
    byte buf[] = { 0x30, 7,
                   0x00, 0x03, 'a', '/', 'b',
                   'H', 'I' };
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    rc = MqttDecode_Publish(buf, (int)sizeof(buf), &pub);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(MQTT_QOS_0, pub.qos);
    ASSERT_EQ(0, pub.packet_id);
    ASSERT_EQ(3, pub.topic_name_len);
    ASSERT_EQ(0, XMEMCMP(pub.topic_name, "a/b", 3));
    ASSERT_EQ(2, (int)pub.total_len);
    ASSERT_EQ(2, (int)pub.buffer_len);
    ASSERT_EQ('H', pub.buffer[0]);
    ASSERT_EQ('I', pub.buffer[1]);
}

TEST(decode_publish_qos1_valid)
{
    /* Fixed header (PUBLISH | QoS 1 = 0x32, remain_len=7),
     * topic "t", packet_id=42, payload "xy". */
    byte buf[] = { 0x32, 7,
                   0x00, 0x01, 't',
                   0x00, 0x2A,
                   'x', 'y' };
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    rc = MqttDecode_Publish(buf, (int)sizeof(buf), &pub);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(MQTT_QOS_1, pub.qos);
    ASSERT_EQ(42, pub.packet_id);
    ASSERT_EQ(1, pub.topic_name_len);
    ASSERT_EQ(0, XMEMCMP(pub.topic_name, "t", 1));
    ASSERT_EQ(2, (int)pub.total_len);
    ASSERT_EQ(2, (int)pub.buffer_len);
    ASSERT_EQ('x', pub.buffer[0]);
    ASSERT_EQ('y', pub.buffer[1]);
}

/* Zero-payload PUBLISH is valid per spec; catches a
 * variable_len>remain_len -> variable_len>=remain_len mutation. */
TEST(decode_publish_qos0_zero_payload)
{
    byte buf[] = { 0x30, 3,
                   0x00, 0x01, 'a' };
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    rc = MqttDecode_Publish(buf, (int)sizeof(buf), &pub);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(MQTT_QOS_0, pub.qos);
    ASSERT_EQ(1, pub.topic_name_len);
    ASSERT_EQ(0, (int)pub.total_len);
    ASSERT_EQ(0, (int)pub.buffer_len);
}

/* Fixed header claims remain_len=3, but topic declares length=5
 * (consuming 7 bytes of variable header). After decoding the topic,
 * variable_len (7) exceeds remain_len (3), which must be rejected. */
TEST(decode_publish_malformed_variable_exceeds_remain)
{
    byte buf[] = { 0x30, 3,
                   0x00, 0x05, 'h', 'e', 'l', 'l', 'o' };
    MqttPublish pub;
    int rc;

    XMEMSET(&pub, 0, sizeof(pub));
    rc = MqttDecode_Publish(buf, (int)sizeof(buf), &pub);
    ASSERT_EQ(MQTT_CODE_ERROR_OUT_OF_BUFFER, rc);
}

#ifdef WOLFMQTT_V5
TEST(decode_publish_v5_with_props_roundtrip)
{
    byte buf[256];
    byte payload[] = { 'p', 'a', 'y' };
    MqttPublish enc, dec;
    MqttProp prop;
    char content_type[] = "text/plain";
    int enc_len, dec_len;

    XMEMSET(&enc, 0, sizeof(enc));
    XMEMSET(&prop, 0, sizeof(prop));
    prop.type = MQTT_PROP_CONTENT_TYPE;
    prop.data_str.str = content_type;
    prop.data_str.len = (word16)XSTRLEN(content_type);
    prop.next = NULL;
    enc.topic_name = "v5/topic";
    enc.qos = MQTT_QOS_1;
    enc.packet_id = 7;
    enc.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    enc.props = &prop;
    enc.buffer = payload;
    enc.total_len = sizeof(payload);

    enc_len = MqttEncode_Publish(buf, (int)sizeof(buf), &enc, 0);
    ASSERT_TRUE(enc_len > 0);

    XMEMSET(&dec, 0, sizeof(dec));
    dec.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    dec_len = MqttDecode_Publish(buf, enc_len, &dec);
    ASSERT_TRUE(dec_len > 0);
    ASSERT_EQ(MQTT_QOS_1, dec.qos);
    ASSERT_EQ(7, dec.packet_id);
    ASSERT_EQ((int)XSTRLEN("v5/topic"), (int)dec.topic_name_len);
    ASSERT_EQ(0, XMEMCMP(dec.topic_name, "v5/topic",
                         XSTRLEN("v5/topic")));
    ASSERT_EQ((int)sizeof(payload), (int)dec.total_len);
    ASSERT_TRUE(dec.props != NULL);
    if (dec.props) {
        MqttProps_Free(dec.props);
    }
}
#endif /* WOLFMQTT_V5 */

/* ============================================================================
 * MqttDecode_ConnectAck
 * ============================================================================ */

TEST(decode_connack_valid)
{
    byte buf[4];
    MqttConnectAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK);
    buf[1] = 2;
    buf[2] = 0;
    buf[3] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_ConnectAck(buf, 4, &ack);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(0, ack.return_code);
}

TEST(decode_connack_malformed_remain_len_zero)
{
    byte buf[2];
    MqttConnectAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK);
    buf[1] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_ConnectAck(buf, 2, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_connack_malformed_remain_len_one)
{
    byte buf[3];
    MqttConnectAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK);
    buf[1] = 1;
    buf[2] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_ConnectAck(buf, 3, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

/* ============================================================================
 * MqttEncode_Subscribe
 * ============================================================================ */

TEST(encode_subscribe_packet_id_zero)
{
    byte tx_buf[256];
    MqttSubscribe sub;
    MqttTopic topic;
    int rc;

    XMEMSET(&sub, 0, sizeof(sub));
    XMEMSET(&topic, 0, sizeof(topic));
    topic.topic_filter = "test/topic";
    sub.topics = &topic;
    sub.topic_count = 1;
    sub.packet_id = 0;
    rc = MqttEncode_Subscribe(tx_buf, (int)sizeof(tx_buf), &sub);
    ASSERT_EQ(MQTT_CODE_ERROR_PACKET_ID, rc);
}

TEST(encode_subscribe_valid)
{
    byte tx_buf[256];
    MqttSubscribe sub;
    MqttTopic topic;
    int rc;

    XMEMSET(&sub, 0, sizeof(sub));
    XMEMSET(&topic, 0, sizeof(topic));
    topic.topic_filter = "test/topic";
    sub.topics = &topic;
    sub.topic_count = 1;
    sub.packet_id = 1;
    rc = MqttEncode_Subscribe(tx_buf, (int)sizeof(tx_buf), &sub);
    ASSERT_TRUE(rc > 0);
}

/* ============================================================================
 * MqttEncode_Unsubscribe
 * ============================================================================ */

TEST(encode_unsubscribe_packet_id_zero)
{
    byte tx_buf[256];
    MqttUnsubscribe unsub;
    MqttTopic topic;
    int rc;

    XMEMSET(&unsub, 0, sizeof(unsub));
    XMEMSET(&topic, 0, sizeof(topic));
    topic.topic_filter = "test/topic";
    unsub.topics = &topic;
    unsub.topic_count = 1;
    unsub.packet_id = 0;
    rc = MqttEncode_Unsubscribe(tx_buf, (int)sizeof(tx_buf), &unsub);
    ASSERT_EQ(MQTT_CODE_ERROR_PACKET_ID, rc);
}

TEST(encode_unsubscribe_valid)
{
    byte tx_buf[256];
    MqttUnsubscribe unsub;
    MqttTopic topic;
    int rc;

    XMEMSET(&unsub, 0, sizeof(unsub));
    XMEMSET(&topic, 0, sizeof(topic));
    topic.topic_filter = "test/topic";
    unsub.topics = &topic;
    unsub.topic_count = 1;
    unsub.packet_id = 1;
    rc = MqttEncode_Unsubscribe(tx_buf, (int)sizeof(tx_buf), &unsub);
    ASSERT_TRUE(rc > 0);
}

/* ============================================================================
 * MqttEncode_Connect
 * ============================================================================ */

/* [MQTT-3.1.2-22] Password must not be present without username */
TEST(encode_connect_password_without_username)
{
    byte tx_buf[256];
    MqttConnect conn;
    int rc;

    XMEMSET(&conn, 0, sizeof(conn));
    conn.client_id = "test_client";
    conn.username = NULL;
    conn.password = "secret";
    rc = MqttEncode_Connect(tx_buf, (int)sizeof(tx_buf), &conn);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(encode_connect_username_and_password)
{
    byte tx_buf[256];
    MqttConnect conn;
    int rc;

    XMEMSET(&conn, 0, sizeof(conn));
    conn.client_id = "test_client";
    conn.username = "user";
    conn.password = "secret";
    rc = MqttEncode_Connect(tx_buf, (int)sizeof(tx_buf), &conn);
    ASSERT_TRUE(rc > 0);
}

TEST(encode_connect_username_only)
{
    byte tx_buf[256];
    MqttConnect conn;
    int rc;

    XMEMSET(&conn, 0, sizeof(conn));
    conn.client_id = "test_client";
    conn.username = "user";
    conn.password = NULL;
    rc = MqttEncode_Connect(tx_buf, (int)sizeof(tx_buf), &conn);
    ASSERT_TRUE(rc > 0);
}

TEST(encode_connect_no_credentials)
{
    byte tx_buf[256];
    MqttConnect conn;
    int rc;

    XMEMSET(&conn, 0, sizeof(conn));
    conn.client_id = "test_client";
    conn.username = NULL;
    conn.password = NULL;
    rc = MqttEncode_Connect(tx_buf, (int)sizeof(tx_buf), &conn);
    ASSERT_TRUE(rc > 0);
}

/* ============================================================================
 * QoS 2 next-ack arithmetic (PUBLISH_REC -> REL -> COMP)
 * ============================================================================ */

TEST(qos2_ack_arithmetic)
{
    ASSERT_EQ(MQTT_PACKET_TYPE_PUBLISH_REL,
              MQTT_PACKET_TYPE_PUBLISH_REC + 1);
    ASSERT_EQ(MQTT_PACKET_TYPE_PUBLISH_COMP,
              MQTT_PACKET_TYPE_PUBLISH_REL + 1);
    ASSERT_EQ(5, MQTT_PACKET_TYPE_PUBLISH_REC);
    ASSERT_EQ(6, MQTT_PACKET_TYPE_PUBLISH_REL);
    ASSERT_EQ(7, MQTT_PACKET_TYPE_PUBLISH_COMP);
}

/* ============================================================================
 * MqttDecode_SubscribeAck
 * ============================================================================ */

TEST(decode_suback_valid)
{
    byte buf[5];
    MqttSubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_SUBSCRIBE_ACK);
    buf[1] = 3;
    buf[2] = 0;
    buf[3] = 1;
    buf[4] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_SubscribeAck(buf, 5, &ack);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(1, ack.packet_id);
}

TEST(decode_suback_malformed_remain_len_zero)
{
    byte buf[2];
    MqttSubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_SUBSCRIBE_ACK);
    buf[1] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_SubscribeAck(buf, 2, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_suback_malformed_remain_len_one)
{
    byte buf[3];
    MqttSubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_SUBSCRIBE_ACK);
    buf[1] = 1;
    buf[2] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_SubscribeAck(buf, 3, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

/* ============================================================================
 * MqttDecode_PublishResp
 * ============================================================================ */

TEST(decode_publish_resp_valid)
{
    byte buf[4];
    MqttPublishResp resp;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_PUBLISH_ACK);
    buf[1] = 2;
    buf[2] = 0;
    buf[3] = 1;
    XMEMSET(&resp, 0, sizeof(resp));
    rc = MqttDecode_PublishResp(buf, 4, MQTT_PACKET_TYPE_PUBLISH_ACK, &resp);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(1, resp.packet_id);
}

TEST(decode_publish_resp_malformed_remain_len_zero)
{
    byte buf[2];
    MqttPublishResp resp;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_PUBLISH_ACK);
    buf[1] = 0;
    XMEMSET(&resp, 0, sizeof(resp));
    rc = MqttDecode_PublishResp(buf, 2, MQTT_PACKET_TYPE_PUBLISH_ACK, &resp);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_publish_resp_malformed_remain_len_one)
{
    byte buf[3];
    MqttPublishResp resp;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_PUBLISH_ACK);
    buf[1] = 1;
    buf[2] = 0;
    XMEMSET(&resp, 0, sizeof(resp));
    rc = MqttDecode_PublishResp(buf, 3, MQTT_PACKET_TYPE_PUBLISH_ACK, &resp);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

/* ============================================================================
 * MqttDecode_UnsubscribeAck
 * ============================================================================ */

TEST(decode_unsuback_valid)
{
    byte buf[4];
    MqttUnsubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_UNSUBSCRIBE_ACK);
    buf[1] = 2;
    buf[2] = 0;
    buf[3] = 1;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_UnsubscribeAck(buf, 4, &ack);
    ASSERT_TRUE(rc > 0);
    ASSERT_EQ(1, ack.packet_id);
}

TEST(decode_unsuback_malformed_remain_len_zero)
{
    byte buf[2];
    MqttUnsubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_UNSUBSCRIBE_ACK);
    buf[1] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_UnsubscribeAck(buf, 2, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

TEST(decode_unsuback_malformed_remain_len_one)
{
    byte buf[3];
    MqttUnsubscribeAck ack;
    int rc;

    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_UNSUBSCRIBE_ACK);
    buf[1] = 1;
    buf[2] = 0;
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_UnsubscribeAck(buf, 3, &ack);
    ASSERT_EQ(MQTT_CODE_ERROR_MALFORMED_DATA, rc);
}

/* ============================================================================
 * MqttEncode/Decode_PublishResp v5 roundtrip
 * ============================================================================ */

#ifdef WOLFMQTT_V5
TEST(publish_resp_v5_success_with_props_roundtrip)
{
    byte buf[256];
    MqttPublishResp enc, dec;
    MqttProp prop;
    int enc_len, dec_len;
    char reason_str[] = "ok";

    XMEMSET(&enc, 0, sizeof(enc));
    XMEMSET(&prop, 0, sizeof(prop));
    prop.type = MQTT_PROP_REASON_STR;
    prop.data_str.str = reason_str;
    prop.data_str.len = (word16)XSTRLEN(reason_str);
    prop.next = NULL;
    enc.packet_id = 1;
    enc.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    enc.reason_code = MQTT_REASON_SUCCESS;
    enc.props = &prop;

    enc_len = MqttEncode_PublishResp(buf, (int)sizeof(buf),
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &enc);
    ASSERT_TRUE(enc_len > 0);

    XMEMSET(&dec, 0, sizeof(dec));
    dec.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    dec_len = MqttDecode_PublishResp(buf, enc_len,
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &dec);
    ASSERT_TRUE(dec_len > 0);
    ASSERT_EQ(1, dec.packet_id);
    ASSERT_EQ(MQTT_REASON_SUCCESS, dec.reason_code);
    if (dec.props) {
        MqttProps_Free(dec.props);
    }
}

TEST(publish_resp_v5_error_no_props_roundtrip)
{
    byte buf[256];
    MqttPublishResp enc, dec;
    int enc_len, dec_len;

    XMEMSET(&enc, 0, sizeof(enc));
    enc.packet_id = 2;
    enc.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    enc.reason_code = 0x80;  /* Unspecified error */
    enc.props = NULL;

    enc_len = MqttEncode_PublishResp(buf, (int)sizeof(buf),
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &enc);
    ASSERT_TRUE(enc_len > 0);

    XMEMSET(&dec, 0, sizeof(dec));
    dec.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    dec_len = MqttDecode_PublishResp(buf, enc_len,
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &dec);
    ASSERT_TRUE(dec_len > 0);
    ASSERT_EQ(0x80, dec.reason_code);
}

TEST(publish_resp_v5_success_no_props_roundtrip)
{
    byte buf[256];
    MqttPublishResp enc, dec;
    int enc_len, dec_len;

    XMEMSET(&enc, 0, sizeof(enc));
    enc.packet_id = 3;
    enc.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    enc.reason_code = MQTT_REASON_SUCCESS;
    enc.props = NULL;

    enc_len = MqttEncode_PublishResp(buf, (int)sizeof(buf),
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &enc);
    ASSERT_TRUE(enc_len > 0);

    XMEMSET(&dec, 0, sizeof(dec));
    dec.protocol_level = MQTT_CONNECT_PROTOCOL_LEVEL_5;
    dec_len = MqttDecode_PublishResp(buf, enc_len,
                  MQTT_PACKET_TYPE_PUBLISH_ACK, &dec);
    ASSERT_TRUE(dec_len > 0);
    ASSERT_EQ(MQTT_REASON_SUCCESS, dec.reason_code);
}
#endif /* WOLFMQTT_V5 */

/* ============================================================================
 * Test Suite Runner
 * ============================================================================ */

void run_mqtt_packet_tests(void)
{
#ifdef WOLFMQTT_V5
    MqttProps_Init();
#endif

    TEST_SUITE_BEGIN("mqtt_packet", setup, teardown);

    /* MqttEncode_Num / MqttDecode_Num */
    RUN_TEST(encode_num_basic);
    RUN_TEST(encode_num_zero);
    RUN_TEST(encode_num_max);
    RUN_TEST(encode_num_null_buf);
    RUN_TEST(decode_num_basic);
    RUN_TEST(decode_num_zero);
    RUN_TEST(decode_num_max);
    RUN_TEST(decode_num_buffer_too_small);
    RUN_TEST(encode_decode_num_roundtrip);

    /* MqttEncode_Vbi / MqttDecode_Vbi */
    RUN_TEST(encode_vbi_one_byte);
    RUN_TEST(encode_vbi_two_bytes);
    RUN_TEST(encode_vbi_three_bytes);
    RUN_TEST(encode_vbi_four_bytes);
    RUN_TEST(encode_vbi_null_buf);
    RUN_TEST(decode_vbi_one_byte_zero);
    RUN_TEST(decode_vbi_one_byte_max);
    RUN_TEST(decode_vbi_two_bytes_min);
    RUN_TEST(decode_vbi_two_bytes_max);
    RUN_TEST(decode_vbi_three_bytes_max);
    RUN_TEST(decode_vbi_four_bytes_max);
    RUN_TEST(decode_vbi_five_byte_malformed);
    RUN_TEST(decode_vbi_buffer_too_small);
    RUN_TEST(decode_vbi_overlong_2byte_zero);
    RUN_TEST(decode_vbi_overlong_3byte_zero);
    RUN_TEST(decode_vbi_overlong_4byte_zero);
    RUN_TEST(decode_vbi_overlong_2byte_127);
    RUN_TEST(encode_decode_vbi_roundtrip);

    /* MqttEncode_Publish */
    RUN_TEST(encode_publish_qos1_packet_id_zero);
    RUN_TEST(encode_publish_qos2_packet_id_zero);
    RUN_TEST(encode_publish_qos0_packet_id_zero_ok);
    RUN_TEST(encode_publish_qos1_valid);

    /* MqttDecode_Publish */
    RUN_TEST(decode_publish_qos0_valid);
    RUN_TEST(decode_publish_qos1_valid);
    RUN_TEST(decode_publish_qos0_zero_payload);
    RUN_TEST(decode_publish_malformed_variable_exceeds_remain);
#ifdef WOLFMQTT_V5
    RUN_TEST(decode_publish_v5_with_props_roundtrip);
#endif

    /* MqttDecode_ConnectAck */
    RUN_TEST(decode_connack_valid);
    RUN_TEST(decode_connack_malformed_remain_len_zero);
    RUN_TEST(decode_connack_malformed_remain_len_one);

    /* MqttEncode_Subscribe */
    RUN_TEST(encode_subscribe_packet_id_zero);
    RUN_TEST(encode_subscribe_valid);

    /* MqttEncode_Unsubscribe */
    RUN_TEST(encode_unsubscribe_packet_id_zero);
    RUN_TEST(encode_unsubscribe_valid);

    /* MqttEncode_Connect */
    RUN_TEST(encode_connect_password_without_username);
    RUN_TEST(encode_connect_username_and_password);
    RUN_TEST(encode_connect_username_only);
    RUN_TEST(encode_connect_no_credentials);

    /* QoS 2 ack arithmetic */
    RUN_TEST(qos2_ack_arithmetic);

    /* MqttDecode_SubscribeAck */
    RUN_TEST(decode_suback_valid);
    RUN_TEST(decode_suback_malformed_remain_len_zero);
    RUN_TEST(decode_suback_malformed_remain_len_one);

    /* MqttDecode_PublishResp */
    RUN_TEST(decode_publish_resp_valid);
    RUN_TEST(decode_publish_resp_malformed_remain_len_zero);
    RUN_TEST(decode_publish_resp_malformed_remain_len_one);

    /* MqttDecode_UnsubscribeAck */
    RUN_TEST(decode_unsuback_valid);
    RUN_TEST(decode_unsuback_malformed_remain_len_zero);
    RUN_TEST(decode_unsuback_malformed_remain_len_one);

#ifdef WOLFMQTT_V5
    RUN_TEST(publish_resp_v5_success_with_props_roundtrip);
    RUN_TEST(publish_resp_v5_error_no_props_roundtrip);
    RUN_TEST(publish_resp_v5_success_no_props_roundtrip);
#endif

    TEST_SUITE_END();

#ifdef WOLFMQTT_V5
    MqttProps_ShutDown();
#endif
}
