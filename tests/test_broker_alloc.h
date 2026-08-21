/* test_broker_alloc.h
 *
 * Copyright (C) 2006-2026 wolfSSL Inc.
 *
 * This file is part of wolfMQTT.
 */

#ifndef WOLFMQTT_TEST_BROKER_ALLOC_H
#define WOLFMQTT_TEST_BROKER_ALLOC_H

#include <stddef.h>

void* wolfmqtt_test_broker_malloc(size_t size);
void wolfmqtt_test_broker_free(void* ptr);

#define WOLFMQTT_MALLOC(size) wolfmqtt_test_broker_malloc((size))
#define WOLFMQTT_FREE(ptr)    wolfmqtt_test_broker_free((ptr))

#endif /* WOLFMQTT_TEST_BROKER_ALLOC_H */
