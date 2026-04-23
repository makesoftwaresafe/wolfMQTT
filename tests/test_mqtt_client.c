/* test_mqtt_client.c
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

#include "wolfmqtt/mqtt_client.h"
#include "tests/unit_test.h"

void run_mqtt_client_tests(void);

/* ============================================================================
 * Test Fixtures
 * ============================================================================ */

#define TEST_TX_BUF_SIZE 256
#define TEST_RX_BUF_SIZE 256
#define TEST_CMD_TIMEOUT_MS 1000

static MqttClient test_client;
static MqttNet test_net;
static byte test_tx_buf[TEST_TX_BUF_SIZE];
static byte test_rx_buf[TEST_RX_BUF_SIZE];

/* Mock network callbacks - just return errors since we're not actually
 * connecting to anything */
static int mock_net_connect(void *context, const char* host, word16 port,
    int timeout_ms)
{
    (void)context; (void)host; (void)port; (void)timeout_ms;
    return MQTT_CODE_ERROR_NETWORK;
}

static int mock_net_read(void *context, byte* buf, int buf_len, int timeout_ms)
{
    (void)context; (void)buf; (void)buf_len; (void)timeout_ms;
    return MQTT_CODE_ERROR_NETWORK;
}

static int mock_net_write(void *context, const byte* buf, int buf_len,
    int timeout_ms)
{
    (void)context; (void)buf; (void)buf_len; (void)timeout_ms;
    return MQTT_CODE_ERROR_NETWORK;
}

static int mock_net_disconnect(void *context)
{
    (void)context;
    return MQTT_CODE_SUCCESS;
}

static int test_client_inited;

static void setup(void)
{
    XMEMSET(&test_client, 0, sizeof(test_client));
    XMEMSET(&test_net, 0, sizeof(test_net));
    XMEMSET(test_tx_buf, 0, sizeof(test_tx_buf));
    XMEMSET(test_rx_buf, 0, sizeof(test_rx_buf));
    test_client_inited = 0;

    /* Setup mock network callbacks */
    test_net.connect = mock_net_connect;
    test_net.read = mock_net_read;
    test_net.write = mock_net_write;
    test_net.disconnect = mock_net_disconnect;
}

static void teardown(void)
{
    /* Only DeInit if Init succeeded — DeInit calls MqttProps_ShutDown
     * which decrements a ref counter that must be balanced with Init. */
    if (test_client_inited) {
        MqttClient_DeInit(&test_client);
    }
}

static int test_init_client(void)
{
    int rc = MqttClient_Init(&test_client, &test_net, NULL,
                             test_tx_buf, TEST_TX_BUF_SIZE,
                             test_rx_buf, TEST_RX_BUF_SIZE,
                             TEST_CMD_TIMEOUT_MS);
    if (rc == MQTT_CODE_SUCCESS) {
        test_client_inited = 1;
    }
    return rc;
}

/* ============================================================================
 * MqttClient_Init Tests
 * ============================================================================ */

TEST(init_null_client)
{
    int rc;

    rc = MqttClient_Init(NULL, &test_net, NULL,
                         test_tx_buf, TEST_TX_BUF_SIZE,
                         test_rx_buf, TEST_RX_BUF_SIZE,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_null_tx_buf)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         NULL, TEST_TX_BUF_SIZE,
                         test_rx_buf, TEST_RX_BUF_SIZE,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_zero_tx_buf_len)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         test_tx_buf, 0,
                         test_rx_buf, TEST_RX_BUF_SIZE,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_null_rx_buf)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         test_tx_buf, TEST_TX_BUF_SIZE,
                         NULL, TEST_RX_BUF_SIZE,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_zero_rx_buf_len)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         test_tx_buf, TEST_TX_BUF_SIZE,
                         test_rx_buf, 0,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_success)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    /* Verify client structure is set up correctly */
    ASSERT_TRUE(test_client.tx_buf == test_tx_buf);
    ASSERT_EQ(TEST_TX_BUF_SIZE, test_client.tx_buf_len);
    ASSERT_TRUE(test_client.rx_buf == test_rx_buf);
    ASSERT_EQ(TEST_RX_BUF_SIZE, test_client.rx_buf_len);
    ASSERT_EQ(TEST_CMD_TIMEOUT_MS, test_client.cmd_timeout_ms);
}

TEST(init_negative_tx_buf_len)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         test_tx_buf, -1,
                         test_rx_buf, TEST_RX_BUF_SIZE,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(init_negative_rx_buf_len)
{
    int rc;

    rc = MqttClient_Init(&test_client, &test_net, NULL,
                         test_tx_buf, TEST_TX_BUF_SIZE,
                         test_rx_buf, -1,
                         TEST_CMD_TIMEOUT_MS);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_DeInit Tests
 * ============================================================================ */

TEST(deinit_null_client)
{
    /* MqttClient_DeInit(NULL) still calls MqttProps_ShutDown() under
     * WOLFMQTT_V5, which decrements a refcount. Pair it with MqttProps_Init()
     * so the refcount stays balanced across test runs. */
#ifdef WOLFMQTT_V5
    (void)MqttProps_Init();
#endif
    /* Should not crash with NULL client */
    MqttClient_DeInit(NULL);
    /* If we reach here, test passes */
    ASSERT_TRUE(1);
}

TEST(deinit_after_init)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    /* DeInit should not crash */
    MqttClient_DeInit(&test_client);
    test_client_inited = 0;
    ASSERT_TRUE(1);
}

/* ============================================================================
 * MqttClient_Connect Tests
 * ============================================================================ */

TEST(connect_null_client)
{
    int rc;
    MqttConnect connect;

    XMEMSET(&connect, 0, sizeof(connect));

    rc = MqttClient_Connect(NULL, &connect);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(connect_null_connect)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    rc = MqttClient_Connect(&test_client, NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(connect_both_null)
{
    int rc;

    rc = MqttClient_Connect(NULL, NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(connect_with_mock_network)
{
    int rc;
    MqttConnect connect;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    XMEMSET(&connect, 0, sizeof(connect));
    connect.keep_alive_sec = 60;
    connect.clean_session = 1;
    connect.client_id = "test_client";

    /* Connect will fail at the network write stage since mock returns error */
    rc = MqttClient_Connect(&test_client, &connect);
    /* Should fail with network error since mock network write returns error */
    ASSERT_EQ(MQTT_CODE_ERROR_NETWORK, rc);
}

/* Regression test for tx_buf credential zeroing after CONNECT is sent.
 * Guards CLIENT_FORCE_ZERO(client->tx_buf, xfer) in MqttClient_Connect: the
 * original issue being prevented is plaintext credentials lingering in the
 * client's tx_buf after the CONNECT packet is written. Without this test, a
 * regression that deletes that line (or passes length 0) would pass silently. */
#define TEST_CONNECT_USERNAME "user"
#define TEST_CONNECT_PASSWORD "secret"

static int connect_mock_xfer;
static byte connect_mock_sent[TEST_TX_BUF_SIZE];

static int mock_net_write_accept(void *context, const byte* buf, int buf_len,
    int timeout_ms)
{
    (void)context; (void)timeout_ms;
    if (buf != NULL && buf_len > 0 &&
        buf_len <= (int)sizeof(connect_mock_sent)) {
        XMEMCPY(connect_mock_sent, buf, (size_t)buf_len);
        connect_mock_xfer = buf_len;
    }
    /* Pretend the full packet was sent so MqttClient_Connect reaches the
     * CLIENT_FORCE_ZERO step. */
    return buf_len;
}

static int buf_contains(const byte* buf, int buf_len,
    const char* needle, int needle_len)
{
    int i;
    if (buf == NULL || needle_len <= 0 || buf_len < needle_len) {
        return 0;
    }
    for (i = 0; i + needle_len <= buf_len; i++) {
        if (XMEMCMP(&buf[i], needle, (size_t)needle_len) == 0) {
            return 1;
        }
    }
    return 0;
}

TEST(connect_clears_tx_buf_credentials)
{
    int rc;
    int i;
    MqttConnect connect;
    const int user_len = (int)sizeof(TEST_CONNECT_USERNAME) - 1;
    const int pass_len = (int)sizeof(TEST_CONNECT_PASSWORD) - 1;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    /* Swap in a write mock that accepts the packet and records what was
     * sent. Read still returns MQTT_CODE_ERROR_NETWORK so MqttClient_Connect
     * returns after the CLIENT_FORCE_ZERO step. */
    connect_mock_xfer = 0;
    XMEMSET(connect_mock_sent, 0, sizeof(connect_mock_sent));
    test_net.write = mock_net_write_accept;

    XMEMSET(&connect, 0, sizeof(connect));
    connect.keep_alive_sec = 60;
    connect.clean_session = 1;
    connect.client_id = "test_client";
    connect.username = TEST_CONNECT_USERNAME;
    connect.password = TEST_CONNECT_PASSWORD;

    rc = MqttClient_Connect(&test_client, &connect);
    /* The read mock cannot deliver a CONNECT_ACK, so a successful return
     * would be wrong regardless of the zeroing step. */
    ASSERT_NE(MQTT_CODE_SUCCESS, rc);

    /* Confirm the write path actually ran with credentials present. Without
     * this, the zeroing assertion below could pass trivially. */
    ASSERT_TRUE(connect_mock_xfer > 0);
    ASSERT_TRUE(buf_contains(connect_mock_sent, connect_mock_xfer,
                             TEST_CONNECT_USERNAME, user_len));
    ASSERT_TRUE(buf_contains(connect_mock_sent, connect_mock_xfer,
                             TEST_CONNECT_PASSWORD, pass_len));

    /* Core regression check: credentials must not remain in tx_buf after
     * MqttClient_Connect returns. Scans the full buffer because the zeroed
     * region covers [0..xfer) and the remainder was zero-initialized at
     * setup. */
    ASSERT_FALSE(buf_contains(test_client.tx_buf, TEST_TX_BUF_SIZE,
                              TEST_CONNECT_USERNAME, user_len));
    ASSERT_FALSE(buf_contains(test_client.tx_buf, TEST_TX_BUF_SIZE,
                              TEST_CONNECT_PASSWORD, pass_len));

    /* Stronger boundary check: every byte the mock observed being written
     * must now be zero. This catches both deletion of the CLIENT_FORCE_ZERO
     * call and an `xfer` -> `0` mutation that turns the wipe into a no-op. */
    for (i = 0; i < connect_mock_xfer; i++) {
        if (test_client.tx_buf[i] != 0) {
            FAIL("tx_buf byte within xfer range is non-zero after CONNECT");
        }
    }
}

/* ============================================================================
 * MqttClient_Disconnect Tests
 * ============================================================================ */

TEST(disconnect_null_client)
{
    int rc;

    rc = MqttClient_Disconnect(NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_GetProtocolVersion Tests
 * ============================================================================ */

TEST(get_protocol_version_default)
{
    int rc;
    int version;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    version = MqttClient_GetProtocolVersion(&test_client);
    /* Default protocol version should be 4 (v3.1.1) or 5 (v5.0) depending
     * on build options */
#ifdef WOLFMQTT_V5
    ASSERT_EQ(MQTT_CONNECT_PROTOCOL_LEVEL_5, version);
#else
    ASSERT_EQ(MQTT_CONNECT_PROTOCOL_LEVEL_4, version);
#endif
}

TEST(get_protocol_version_string)
{
    int rc;
    const char* version_str;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    version_str = MqttClient_GetProtocolVersionString(&test_client);
    ASSERT_NOT_NULL(version_str);
    /* Should be "v3.1.1" or "v5" depending on build options */
#ifdef WOLFMQTT_V5
    ASSERT_STR_EQ("v5", version_str);
#else
    ASSERT_STR_EQ("v3.1.1", version_str);
#endif
}

/* ============================================================================
 * MqttClient_Ping Tests
 * ============================================================================ */

TEST(ping_null_client)
{
    int rc;

    rc = MqttClient_Ping(NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_Subscribe Tests
 * ============================================================================ */

TEST(subscribe_null_client)
{
    int rc;
    MqttSubscribe subscribe;

    XMEMSET(&subscribe, 0, sizeof(subscribe));

    rc = MqttClient_Subscribe(NULL, &subscribe);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(subscribe_null_subscribe)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    rc = MqttClient_Subscribe(&test_client, NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_Unsubscribe Tests
 * ============================================================================ */

TEST(unsubscribe_null_client)
{
    int rc;
    MqttUnsubscribe unsubscribe;

    XMEMSET(&unsubscribe, 0, sizeof(unsubscribe));

    rc = MqttClient_Unsubscribe(NULL, &unsubscribe);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(unsubscribe_null_unsubscribe)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    rc = MqttClient_Unsubscribe(&test_client, NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_Publish Tests
 * ============================================================================ */

TEST(publish_null_client)
{
    int rc;
    MqttPublish publish;

    XMEMSET(&publish, 0, sizeof(publish));

    rc = MqttClient_Publish(NULL, &publish);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

TEST(publish_null_publish)
{
    int rc;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    rc = MqttClient_Publish(&test_client, NULL);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_WaitMessage Tests
 * ============================================================================ */

TEST(wait_message_null_client)
{
    int rc;

    rc = MqttClient_WaitMessage(NULL, 1000);
    ASSERT_EQ(MQTT_CODE_ERROR_BAD_ARG, rc);
}

/* ============================================================================
 * MqttClient_ReturnCodeToString Tests
 * ============================================================================ */

#ifndef WOLFMQTT_NO_ERROR_STRINGS
TEST(return_code_to_string_success)
{
    const char* str;

    str = MqttClient_ReturnCodeToString(MQTT_CODE_SUCCESS);
    ASSERT_NOT_NULL(str);
    /* Should contain "Success" or similar */
    ASSERT_TRUE(str[0] != '\0');
}

TEST(return_code_to_string_bad_arg)
{
    const char* str;

    str = MqttClient_ReturnCodeToString(MQTT_CODE_ERROR_BAD_ARG);
    ASSERT_NOT_NULL(str);
    ASSERT_TRUE(str[0] != '\0');
}

TEST(return_code_to_string_network)
{
    const char* str;

    str = MqttClient_ReturnCodeToString(MQTT_CODE_ERROR_NETWORK);
    ASSERT_NOT_NULL(str);
    ASSERT_TRUE(str[0] != '\0');
}
#endif /* WOLFMQTT_NO_ERROR_STRINGS */

/* ============================================================================
 * MqttClient_Flags Tests
 * ============================================================================ */

TEST(client_flags_set_clear)
{
    int rc;
    word32 flags;

    rc = test_init_client();
    ASSERT_EQ(MQTT_CODE_SUCCESS, rc);

    /* Initially no flags should be set */
    flags = MqttClient_Flags(&test_client, 0, 0);
    ASSERT_EQ(0, (int)(flags & MQTT_CLIENT_FLAG_IS_CONNECTED));

    /* Set connected flag */
    flags = MqttClient_Flags(&test_client, 0, MQTT_CLIENT_FLAG_IS_CONNECTED);
    ASSERT_TRUE((flags & MQTT_CLIENT_FLAG_IS_CONNECTED) != 0);

    /* Clear connected flag */
    flags = MqttClient_Flags(&test_client, MQTT_CLIENT_FLAG_IS_CONNECTED, 0);
    ASSERT_EQ(0, (int)(flags & MQTT_CLIENT_FLAG_IS_CONNECTED));
}

/* ============================================================================
 * Test Suite Runner
 * ============================================================================ */

void run_mqtt_client_tests(void)
{
    TEST_SUITE_BEGIN("mqtt_client", setup, teardown);

    /* MqttClient_Init tests */
    RUN_TEST(init_null_client);
    RUN_TEST(init_null_tx_buf);
    RUN_TEST(init_zero_tx_buf_len);
    RUN_TEST(init_null_rx_buf);
    RUN_TEST(init_zero_rx_buf_len);
    RUN_TEST(init_success);
    RUN_TEST(init_negative_tx_buf_len);
    RUN_TEST(init_negative_rx_buf_len);

    /* MqttClient_DeInit tests */
    RUN_TEST(deinit_null_client);
    RUN_TEST(deinit_after_init);

    /* MqttClient_Connect tests */
    RUN_TEST(connect_null_client);
    RUN_TEST(connect_null_connect);
    RUN_TEST(connect_both_null);
    RUN_TEST(connect_with_mock_network);
    RUN_TEST(connect_clears_tx_buf_credentials);

    /* MqttClient_Disconnect tests */
    RUN_TEST(disconnect_null_client);

    /* MqttClient_GetProtocolVersion tests */
    RUN_TEST(get_protocol_version_default);
    RUN_TEST(get_protocol_version_string);

    /* MqttClient_Ping tests */
    RUN_TEST(ping_null_client);

    /* MqttClient_Subscribe tests */
    RUN_TEST(subscribe_null_client);
    RUN_TEST(subscribe_null_subscribe);

    /* MqttClient_Unsubscribe tests */
    RUN_TEST(unsubscribe_null_client);
    RUN_TEST(unsubscribe_null_unsubscribe);

    /* MqttClient_Publish tests */
    RUN_TEST(publish_null_client);
    RUN_TEST(publish_null_publish);

    /* MqttClient_WaitMessage tests */
    RUN_TEST(wait_message_null_client);

#ifndef WOLFMQTT_NO_ERROR_STRINGS
    /* MqttClient_ReturnCodeToString tests */
    RUN_TEST(return_code_to_string_success);
    RUN_TEST(return_code_to_string_bad_arg);
    RUN_TEST(return_code_to_string_network);
#endif

    /* MqttClient_Flags tests */
    RUN_TEST(client_flags_set_clear);

    TEST_SUITE_END();
}
