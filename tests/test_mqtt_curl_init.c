/* Deterministic tests for the libcurl global initialization lifecycle. */

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt/mqtt_socket.h"

static CURLcode test_init_result;
static int init_calls;
static int cleanup_calls;
static int disconnect_calls;

CURLcode wolfmqtt_test_curl_global_init(long flags)
{
    (void)flags;
    init_calls++;
    return test_init_result;
}

void wolfmqtt_test_curl_global_cleanup(void)
{
    cleanup_calls++;
}

word32 MqttClient_Flags(MqttClient *client, word32 mask, word32 flags)
{
    if (client != NULL) {
        client->flags &= ~mask;
        client->flags |= flags;
        return client->flags;
    }
    return 0;
}

static int test_connect(void *context, const char* host, word16 port,
    int timeout_ms)
{
    (void)context;
    (void)host;
    (void)port;
    (void)timeout_ms;
    return MQTT_CODE_SUCCESS;
}

static int test_write(void *context, const byte* buf, int buf_len,
    int timeout_ms)
{
    (void)context;
    (void)buf;
    (void)buf_len;
    (void)timeout_ms;
    return MQTT_CODE_SUCCESS;
}

static int test_read(void *context, byte* buf, int buf_len, int timeout_ms)
{
    (void)context;
    (void)buf;
    (void)buf_len;
    (void)timeout_ms;
    return MQTT_CODE_SUCCESS;
}

static int test_disconnect(void *context)
{
    (void)context;
    disconnect_calls++;
    return MQTT_CODE_SUCCESS;
}

static void reset_state(MqttClient* client, MqttNet* net)
{
    XMEMSET(client, 0, sizeof(*client));
    XMEMSET(net, 0, sizeof(*net));
    net->connect = test_connect;
    net->write = test_write;
    net->read = test_read;
    net->disconnect = test_disconnect;
    test_init_result = CURLE_OK;
    init_calls = 0;
    cleanup_calls = 0;
    disconnect_calls = 0;
}

static int test_init_failure_is_reported(void)
{
    MqttClient client;
    MqttNet net;
    int rc;

    reset_state(&client, &net);
    test_init_result = CURLE_FAILED_INIT;
    rc = MqttSocket_Init(&client, &net);
    if (rc != MQTT_CODE_ERROR_CURL) {
        fprintf(stderr, "curl initialization failure returned %d\n", rc);
        return 1;
    }
    (void)MqttSocket_Disconnect(&client);
    if (init_calls != 1 || cleanup_calls != 0) {
        fprintf(stderr, "failed curl init calls=%d cleanup=%d\n",
            init_calls, cleanup_calls);
        return 1;
    }
    return 0;
}

static int test_success_cleanup_is_idempotent(void)
{
    MqttClient client;
    MqttNet net;
    int rc;

    reset_state(&client, &net);
    rc = MqttSocket_Init(&client, &net);
    if (rc != MQTT_CODE_SUCCESS) {
        fprintf(stderr, "curl initialization returned %d\n", rc);
        return 1;
    }
    (void)MqttSocket_Disconnect(&client);
    (void)MqttSocket_Disconnect(&client);
    if (init_calls != 1 || cleanup_calls != 1 || disconnect_calls != 2) {
        fprintf(stderr, "successful curl init calls=%d cleanup=%d disconnect=%d\n",
            init_calls, cleanup_calls, disconnect_calls);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failures = 0;

    failures += test_init_failure_is_reported();
    failures += test_success_cleanup_is_idempotent();
    if (failures == 0) {
        printf("curl initialization tests passed\n");
    }
    return failures == 0 ? 0 : 1;
}
