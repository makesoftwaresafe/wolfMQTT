/* client.c
 *
 * Copyright (C) 2006-2025 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 *
 * wolfMQTT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "wolfmqtt/mqtt_client.h"
#include "examples/mqttclient/mqttclient.h"

int main(void)
{
    int rc;
    MQTTCtx mqttCtx;

    /* init defaults */
    mqtt_init_ctx(&mqttCtx);

    mqttCtx.test_mode = 1;

    /* Set port as configured in scripts/broker_test/mosquitto.conf */
#if defined(WOLFMQTT_DEFAULT_TLS) && (WOLFMQTT_DEFAULT_TLS == 1)
    mqttCtx.port = 18883;
#else
    mqttCtx.port = 11883;
#endif

    rc = mqttclient_test(&mqttCtx);

    mqtt_free_ctx(&mqttCtx);

    if (rc == 0)
        PRINTF("Zephyr MQTT test passed");

    return (rc == 0) ? 0 : EXIT_FAILURE;
}
