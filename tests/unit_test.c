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

/* Main unit test runner for wolfMQTT */

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

/* Define UNIT_TEST_IMPLEMENTATION to provide storage for global counters */
#define UNIT_TEST_IMPLEMENTATION
#include "tests/unit_test.h"

/* Declare test suite runners */
extern void run_mqtt_packet_tests(void);
extern void run_mqtt_client_tests(void);

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    TEST_RUNNER_BEGIN();

    run_mqtt_packet_tests();
    run_mqtt_client_tests();

    TEST_RUNNER_END();
}
