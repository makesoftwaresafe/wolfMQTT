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

static void reset_sem_state(void)
{
    sem_init_calls = 0;
    sem_free_attempts = 0;
    sem_free_calls = 0;
    sem_invalid_free_calls = 0;
    sem_fail_all = 0;
    sem_fail_on_call = 0;
    sem_fail_free_on_call = 0;
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
    (void)sem;
    return MQTT_CODE_SUCCESS;
}

int wm_SemUnlock(wm_Sem* sem)
{
    (void)sem;
    return MQTT_CODE_SUCCESS;
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

int main(void)
{
    if (test_property_lock_failure() != 0) {
        return 1;
    }
    if (test_partial_client_init() != 0) {
        return 1;
    }
    if (test_failed_client_free_retried() != 0) {
        return 1;
    }
    return 0;
}
