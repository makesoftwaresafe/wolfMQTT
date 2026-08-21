/* Property-lock initialization failure tests with deterministic semaphore
 * injection. */

#include <stdio.h>

#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt/mqtt_packet.h"
#include "wolfmqtt/mqtt_socket.h"

static int sem_init_calls;
static int sem_free_attempts;
static int sem_free_calls;
static int sem_invalid_free_calls;
static int sem_fail_all;
static int sem_fail_on_call;
static int sem_fail_free_on_call;
static wm_Sem* sem_complete_on_lock;
static MqttPendResp* sem_pending_to_complete;
static int sem_pending_completions;
static int ping_read_calls;
static int ping_disconnect_calls;

static void reset_sem_state(void)
{
    sem_init_calls = 0;
    sem_free_attempts = 0;
    sem_free_calls = 0;
    sem_invalid_free_calls = 0;
    sem_fail_all = 0;
    sem_fail_on_call = 0;
    sem_fail_free_on_call = 0;
    sem_complete_on_lock = NULL;
    sem_pending_to_complete = NULL;
    sem_pending_completions = 0;
    ping_read_calls = 0;
    ping_disconnect_calls = 0;
}

int wm_SemInit(wm_Sem* sem)
{
    (void)sem;
    sem_init_calls++;
    if (sem_fail_all || (sem_init_calls == sem_fail_on_call)) {
        return MQTT_CODE_ERROR_SYSTEM;
    }
    sem->initialized = 1;
    return MQTT_CODE_SUCCESS;
}

int wm_SemFree(wm_Sem* sem)
{
    sem_free_attempts++;
    if (sem->initialized == 0) {
        sem_invalid_free_calls++;
        return MQTT_CODE_ERROR_BAD_ARG;
    }
    if (sem_free_attempts == sem_fail_free_on_call) {
        return MQTT_CODE_ERROR_SYSTEM;
    }
    sem->initialized = 0;
    sem_free_calls++;
    return MQTT_CODE_SUCCESS;
}

int wm_SemLock(wm_Sem* sem)
{
    if (sem == sem_complete_on_lock && sem_pending_to_complete != NULL) {
        sem_pending_to_complete->packetDone = 1;
        sem_pending_to_complete->packet_ret = MQTT_CODE_SUCCESS;
        sem_pending_to_complete = NULL;
        sem_pending_completions++;
    }
    return MQTT_CODE_SUCCESS;
}

int wm_SemUnlock(wm_Sem* sem)
{
    (void)sem;
    return MQTT_CODE_SUCCESS;
}

static int ping_net_connect(void* context, const char* host, word16 port,
    int timeout_ms)
{
    (void)context;
    (void)host;
    (void)port;
    (void)timeout_ms;
    return MQTT_CODE_SUCCESS;
}

static int ping_net_read(void* context, byte* buf, int buf_len, int timeout_ms)
{
    (void)context;
    (void)buf;
    (void)buf_len;
    (void)timeout_ms;
    ping_read_calls++;
    return MQTT_CODE_ERROR_TIMEOUT;
}

static int ping_net_write(void* context, const byte* buf, int buf_len,
    int timeout_ms)
{
    (void)context;
    (void)buf;
    (void)timeout_ms;
    return buf_len;
}

static int ping_net_disconnect(void* context)
{
    (void)context;
    ping_disconnect_calls++;
    return MQTT_CODE_SUCCESS;
}

static int test_ping_response_completed_during_read_lock(void)
{
    int rc;
    MqttClient client;
    MqttNet net;
    MqttPing ping;
    byte tx_buf[64];
    byte rx_buf[64];

    reset_sem_state();
    XMEMSET(&net, 0, sizeof(net));
    XMEMSET(&ping, 0, sizeof(ping));
    net.connect = ping_net_connect;
    net.read = ping_net_read;
    net.write = ping_net_write;
    net.disconnect = ping_net_disconnect;

    rc = MqttClient_Init(&client, &net, NULL, tx_buf, sizeof(tx_buf),
        rx_buf, sizeof(rx_buf), 1000);
    if (rc != MQTT_CODE_SUCCESS) {
        fprintf(stderr, "ping-race client initialization returned %d\n", rc);
        return 1;
    }

    /* Model another reader delivering PINGRESP after the waiter's first
     * pending-response check but while that waiter acquires lockRecv. */
    sem_complete_on_lock = &client.lockRecv;
    sem_pending_to_complete = &ping.pendResp;
    rc = MqttClient_Ping_ex(&client, &ping);

    if (rc != MQTT_CODE_SUCCESS || sem_pending_completions != 1 ||
            ping_read_calls != 0 || ping_disconnect_calls != 0) {
        fprintf(stderr,
            "completed ping returned %d, completions=%d reads=%d disconnects=%d\n",
            rc, sem_pending_completions, ping_read_calls,
            ping_disconnect_calls);
        MqttClient_DeInit(&client);
        return 1;
    }

    MqttClient_DeInit(&client);
    return 0;
}

static int test_property_lock_failure(void)
{
    int rc;

    reset_sem_state();
    sem_fail_all = 1;

    rc = MqttProps_Init();
    if (rc != MQTT_CODE_ERROR_SYSTEM) {
        fprintf(stderr, "first property-lock failure returned %d\n", rc);
        return 1;
    }

    rc = MqttProps_Init();
    if (rc != MQTT_CODE_ERROR_SYSTEM) {
        fprintf(stderr, "second property-lock failure returned %d\n", rc);
        return 1;
    }
    if (sem_init_calls != 2) {
        fprintf(stderr, "property lock initialized %d times, expected 2\n",
            sem_init_calls);
        return 1;
    }

    (void)MqttProps_ShutDown();
    (void)MqttProps_ShutDown();
    if (sem_free_calls != 0) {
        fprintf(stderr, "uninitialized property lock freed %d times\n",
            sem_free_calls);
        return 1;
    }

    return 0;
}

static int test_partial_client_init(void)
{
    int rc;
    MqttClient client;
    MqttNet net;
    byte tx_buf[64];
    byte rx_buf[64];

    reset_sem_state();
    XMEMSET(&net, 0, sizeof(net));
    /* Property lock and lockSend succeed; lockRecv fails. */
    sem_fail_on_call = 3;

    rc = MqttClient_Init(&client, &net, NULL, tx_buf, sizeof(tx_buf),
        rx_buf, sizeof(rx_buf), 1000);
    if (rc != MQTT_CODE_ERROR_SYSTEM) {
        fprintf(stderr, "partial client initialization returned %d\n", rc);
        return 1;
    }
    if (sem_init_calls != 3) {
        fprintf(stderr, "client initialized %d semaphores, expected 3\n",
            sem_init_calls);
        return 1;
    }
    if ((sem_free_calls != 2) || (sem_invalid_free_calls != 0)) {
        fprintf(stderr, "partial cleanup freed %d valid and %d invalid locks\n",
            sem_free_calls, sem_invalid_free_calls);
        return 1;
    }

    MqttClient_DeInit(&client);
    if ((sem_free_calls != 2) || (sem_invalid_free_calls != 0)) {
        fprintf(stderr, "repeat cleanup freed %d valid and %d invalid locks\n",
            sem_free_calls, sem_invalid_free_calls);
        return 1;
    }

    return 0;
}

static int test_failed_client_free_retried(void)
{
    int rc;
    MqttClient client;
    MqttNet net;
    byte tx_buf[64];
    byte rx_buf[64];

    reset_sem_state();
    XMEMSET(&net, 0, sizeof(net));

    /* Let all locks initialize, then make socket validation fail so cleanup
     * runs; fail the first lock free while leaving that injected lock live. */
    sem_fail_free_on_call = 1;
    rc = MqttClient_Init(&client, &net, NULL, tx_buf, sizeof(tx_buf),
        rx_buf, sizeof(rx_buf), 1000);
    if (rc != MQTT_CODE_ERROR_BAD_ARG) {
        fprintf(stderr, "socket initialization failure returned %d\n", rc);
        return 1;
    }

    sem_fail_free_on_call = 0;
    MqttClient_DeInit(&client);

    if ((sem_free_attempts != 5) || (sem_free_calls != 4) ||
            (sem_invalid_free_calls != 0)) {
        fprintf(stderr,
            "free retry made %d attempts, %d successes and %d invalid frees\n",
            sem_free_attempts, sem_free_calls, sem_invalid_free_calls);
        return 1;
    }

    return 0;
}

static int test_repeated_client_deinit_preserves_property_owner(void)
{
    int rc;
    MqttClient client;
    MqttNet net;
    byte tx_buf[64];
    byte rx_buf[64];

    reset_sem_state();
    XMEMSET(&net, 0, sizeof(net));

    /* Hold one property reference on behalf of another live client. */
    rc = MqttProps_Init();
    if (rc != MQTT_CODE_SUCCESS) {
        fprintf(stderr, "existing property owner initialization returned %d\n",
            rc);
        return 1;
    }

    /* This client initializes its locks, then fails socket validation and
     * performs its own cleanup before the explicit second deinit below. */
    rc = MqttClient_Init(&client, &net, NULL, tx_buf, sizeof(tx_buf),
        rx_buf, sizeof(rx_buf), 1000);
    if (rc != MQTT_CODE_ERROR_BAD_ARG) {
        fprintf(stderr, "failed client initialization returned %d\n", rc);
        (void)MqttProps_ShutDown();
        return 1;
    }
    MqttClient_DeInit(&client);

    if (sem_free_calls != 3) {
        fprintf(stderr, "repeat deinit freed another client's property lock\n");
        (void)MqttProps_ShutDown();
        return 1;
    }

    (void)MqttProps_ShutDown();
    if (sem_free_calls != 4) {
        fprintf(stderr, "property owner cleanup freed %d locks, expected 4\n",
            sem_free_calls);
        return 1;
    }

    return 0;
}

static int test_failed_property_free_retried(void)
{
    int rc;

    reset_sem_state();
    rc = MqttProps_Init();
    if (rc != MQTT_CODE_SUCCESS) {
        fprintf(stderr, "property initialization returned %d\n", rc);
        return 1;
    }

    sem_fail_free_on_call = 1;
    rc = MqttProps_ShutDown();
    if (rc != MQTT_CODE_ERROR_SYSTEM) {
        fprintf(stderr, "property free failure returned %d\n", rc);
        return 1;
    }
    sem_fail_free_on_call = 0;
    rc = MqttProps_ShutDown();
    if ((rc != MQTT_CODE_SUCCESS) || (sem_free_attempts != 2) ||
            (sem_free_calls != 1)) {
        fprintf(stderr, "property free retry made %d attempts and %d frees\n",
            sem_free_attempts, sem_free_calls);
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_ping_response_completed_during_read_lock() != 0) {
        return 1;
    }
    if (test_property_lock_failure() != 0) {
        return 1;
    }
    if (test_partial_client_init() != 0) {
        return 1;
    }
    if (test_failed_client_free_retried() != 0) {
        return 1;
    }
    if (test_failed_property_free_retried() != 0) {
        return 1;
    }
    if (test_repeated_client_deinit_preserves_property_owner() != 0) {
        return 1;
    }
    return 0;
}
