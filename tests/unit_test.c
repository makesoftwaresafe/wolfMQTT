/* unit_test.c
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

/* Include the autoconf generated config.h */
#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include "wolfmqtt/mqtt_packet.h"
#include "wolfmqtt/mqtt_client.h"

static int test_count = 0;
static int fail_count = 0;

#define CHECK(cond, msg) do { \
    test_count++; \
    if (!(cond)) { \
        PRINTF("FAIL: %s (line %d)", msg, __LINE__); \
        fail_count++; \
    } else { \
        PRINTF("  ok: %s", msg); \
    } \
} while (0)

/* -------------------------------------------------------------------------- */
/* MqttDecode_Vbi / MqttEncode_Vbi tests                                     */
/* -------------------------------------------------------------------------- */
static void test_vbi(void)
{
    byte buf[8];
    word32 value;
    int rc;

    PRINTF("--- MqttDecode_Vbi / MqttEncode_Vbi ---");

    /* 1-byte VBI: value 0 */
    buf[0] = 0x00;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 1, "1-byte VBI (0): rc == 1");
    CHECK(value == 0, "1-byte VBI (0): value == 0");

    /* 1-byte VBI: value 127 */
    buf[0] = 0x7F;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 1, "1-byte VBI (127): rc == 1");
    CHECK(value == 127, "1-byte VBI (127): value == 127");

    /* 2-byte VBI: value 128 (0x80 0x01) */
    buf[0] = 0x80; buf[1] = 0x01;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 2, "2-byte VBI (128): rc == 2");
    CHECK(value == 128, "2-byte VBI (128): value == 128");

    /* 2-byte VBI: value 200 (0xC8 0x01) */
    buf[0] = 0xC8; buf[1] = 0x01;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 2, "2-byte VBI (200): rc == 2");
    CHECK(value == 200, "2-byte VBI (200): value == 200");

    /* 2-byte VBI: value 16383 (0xFF 0x7F) - max 2-byte */
    buf[0] = 0xFF; buf[1] = 0x7F;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 2, "2-byte VBI (16383 max): rc == 2");
    CHECK(value == 16383, "2-byte VBI (16383 max): value == 16383");

    /* 3-byte VBI: value 2,097,151 = 0xFF 0xFF 0x7F - max 3-byte */
    buf[0] = 0xFF; buf[1] = 0xFF; buf[2] = 0x7F;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 3, "3-byte VBI (2097151 max): rc == 3");
    CHECK(value == 2097151, "3-byte VBI (2097151 max): value == 2097151");

    /* 4-byte VBI: value 268,435,455 = 0xFF 0xFF 0xFF 0x7F (max MQTT) */
    buf[0] = 0xFF; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0x7F;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 4, "4-byte VBI (268435455 max): rc == 4");
    CHECK(value == 268435455, "4-byte VBI (268435455 max): value correct");

    /* 5-byte malformed: all continuation bits set (5th byte needed) */
    buf[0] = 0xFF; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0xFF; buf[4] = 0x00;
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == MQTT_CODE_ERROR_MALFORMED_DATA,
          "5-byte malformed VBI: returns MALFORMED_DATA");

    /* Buffer too small for multi-byte VBI */
    buf[0] = 0x80;
    rc = MqttDecode_Vbi(buf, &value, 1);
    CHECK(rc == MQTT_CODE_ERROR_OUT_OF_BUFFER,
          "VBI buffer too small: returns OUT_OF_BUFFER");

    /* Encode/Decode roundtrip: max value 268,435,455 */
    rc = MqttEncode_Vbi(buf, 268435455);
    CHECK(rc == 4, "Encode max VBI: 4 bytes");
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 4, "Decode max VBI roundtrip: rc == 4");
    CHECK(value == 268435455, "Decode max VBI roundtrip: value correct");

    /* Encode/Decode roundtrip: 2-byte boundary 128 */
    rc = MqttEncode_Vbi(buf, 128);
    CHECK(rc == 2, "Encode VBI 128: 2 bytes");
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 2, "Decode VBI 128 roundtrip: rc == 2");
    CHECK(value == 128, "Decode VBI 128 roundtrip: value correct");

    /* Encode/Decode roundtrip: 3-byte boundary 16384 */
    rc = MqttEncode_Vbi(buf, 16384);
    CHECK(rc == 3, "Encode VBI 16384: 3 bytes");
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 3, "Decode VBI 16384 roundtrip: rc == 3");
    CHECK(value == 16384, "Decode VBI 16384 roundtrip: value correct");

    /* Encode/Decode roundtrip: 4-byte boundary 2097152 */
    rc = MqttEncode_Vbi(buf, 2097152);
    CHECK(rc == 4, "Encode VBI 2097152: 4 bytes");
    rc = MqttDecode_Vbi(buf, &value, sizeof(buf));
    CHECK(rc == 4, "Decode VBI 2097152 roundtrip: rc == 4");
    CHECK(value == 2097152, "Decode VBI 2097152 roundtrip: value correct");
}

/* -------------------------------------------------------------------------- */
/* MqttEncode_Publish tests                                                   */
/* -------------------------------------------------------------------------- */
static void test_encode_publish(void)
{
    byte tx_buf[256];
    MqttPublish pub;
    int rc;

    PRINTF("--- MqttEncode_Publish ---");

    /* QoS 1 with packet_id == 0 must fail */
    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_1;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    CHECK(rc == MQTT_CODE_ERROR_PACKET_ID,
          "QoS 1 packet_id=0: returns ERROR_PACKET_ID");

    /* QoS 2 with packet_id == 0 must fail */
    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_2;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    CHECK(rc == MQTT_CODE_ERROR_PACKET_ID,
          "QoS 2 packet_id=0: returns ERROR_PACKET_ID");

    /* QoS 0 with packet_id == 0 must succeed (no packet_id needed) */
    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_0;
    pub.packet_id = 0;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    CHECK(rc > 0, "QoS 0 packet_id=0: succeeds");

    /* QoS 1 with valid packet_id must succeed */
    XMEMSET(&pub, 0, sizeof(pub));
    pub.topic_name = "test/topic";
    pub.qos = MQTT_QOS_1;
    pub.packet_id = 1;
    rc = MqttEncode_Publish(tx_buf, (int)sizeof(tx_buf), &pub, 0);
    CHECK(rc > 0, "QoS 1 packet_id=1: succeeds");
}

/* -------------------------------------------------------------------------- */
/* MqttDecode_ConnectAck tests                                                */
/* -------------------------------------------------------------------------- */
static void test_decode_connack(void)
{
    byte buf[8];
    MqttConnectAck ack;
    int rc;

    PRINTF("--- MqttDecode_ConnectAck ---");

    /* Valid CONNACK: remain_len=2, flags=0, return_code=0 */
    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK); /* 0x20 */
    buf[1] = 2;    /* remain_len */
    buf[2] = 0;    /* flags */
    buf[3] = 0;    /* return_code (success) */
    XMEMSET(&ack, 0, sizeof(ack));
    rc = MqttDecode_ConnectAck(buf, 4, &ack);
    CHECK(rc > 0, "CONNACK remain_len=2: succeeds");
    CHECK(ack.return_code == 0, "CONNACK remain_len=2: return_code == 0");

    /* Malformed CONNACK: remain_len=0 */
    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK);
    buf[1] = 0;    /* remain_len = 0 */
    rc = MqttDecode_ConnectAck(buf, 2, NULL);
    CHECK(rc == MQTT_CODE_ERROR_MALFORMED_DATA,
          "CONNACK remain_len=0: returns MALFORMED_DATA");

    /* Malformed CONNACK: remain_len=1 */
    buf[0] = MQTT_PACKET_TYPE_SET(MQTT_PACKET_TYPE_CONNECT_ACK);
    buf[1] = 1;    /* remain_len = 1 */
    buf[2] = 0;    /* only flags, missing return_code */
    rc = MqttDecode_ConnectAck(buf, 3, NULL);
    CHECK(rc == MQTT_CODE_ERROR_MALFORMED_DATA,
          "CONNACK remain_len=1: returns MALFORMED_DATA");
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    PRINTF("wolfMQTT Unit Tests");

    test_vbi();
    test_encode_publish();
    test_decode_connack();

    PRINTF("=== Results: %d/%d passed ===",
           test_count - fail_count, test_count);

    return fail_count > 0 ? 1 : 0;
}
