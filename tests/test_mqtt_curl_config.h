/* Rename libcurl's process-global lifecycle functions so the dedicated socket
 * test can deterministically drive both initialization outcomes. */
#define curl_global_init wolfmqtt_test_curl_global_init
#define curl_global_cleanup wolfmqtt_test_curl_global_cleanup
