/* Property-lock initialization failure tests with deterministic semaphore
 * injection. */

#include <stdio.h>

#include "wolfmqtt/mqtt_client.h"
#include "wolfmqtt/mqtt_packet.h"
#include "wolfmqtt/mqtt_socket.h"

static int sem_init_calls;
static int sem_free_calls;

int wm_SemInit(wm_Sem* sem)
{
    (void)sem;
    sem_init_calls++;
    return MQTT_CODE_ERROR_SYSTEM;
}

int wm_SemFree(wm_Sem* sem)
{
    (void)sem;
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

int MqttSocket_Write(MqttClient* client, const byte* buf, int buf_len,
    int timeout_ms)
{
    (void)client;
    (void)buf;
    (void)buf_len;
    (void)timeout_ms;
    return MQTT_CODE_ERROR_NETWORK;
}

int MqttSocket_Read(MqttClient* client, byte* buf, int buf_len,
    int timeout_ms)
{
    (void)client;
    (void)buf;
    (void)buf_len;
    (void)timeout_ms;
    return MQTT_CODE_ERROR_NETWORK;
}

int main(void)
{
    int rc;

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
