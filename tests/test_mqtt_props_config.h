/* Force the static, multithreaded property implementation for the dedicated
 * property-lock lifecycle test, independent of the main configured build. */
#include "wolfmqtt/options.h"

#ifndef WOLFMQTT_V5
    #define WOLFMQTT_V5
#endif
#ifndef WOLFMQTT_MULTITHREAD
    #define WOLFMQTT_MULTITHREAD
#endif
#ifndef WOLFMQTT_USER_THREADING
    typedef struct TestMqttSem {
        int initialized;
    } wm_Sem;
    #define WOLFMQTT_USER_THREADING
#endif
#ifdef WOLFMQTT_DYN_PROP
    #undef WOLFMQTT_DYN_PROP
#endif
