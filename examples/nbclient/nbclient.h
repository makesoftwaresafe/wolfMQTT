/* nbclient.h
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

#ifndef WOLFMQTT_NBCLIENT_H
#define WOLFMQTT_NBCLIENT_H

#include "examples/mqttexample.h"


/* Exposed functions */
int mqttclient_test(MQTTCtx *mqttCtx);

#if defined(NO_MAIN_DRIVER)
int mqttclient_main(int argc, char** argv);
#endif

#endif /* WOLFMQTT_NBCLIENT_H */
