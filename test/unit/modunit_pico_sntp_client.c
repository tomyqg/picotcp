#include "pico_sntp_client.h"
#include "modules/pico_sntp_client.c"
#include "check.h"

/* Mocking functions, variables, ... */
volatile pico_time pico_tick = 0ull;
volatile pico_err_t pico_err = 0;

/* Used in dnsCallback */
struct pico_socket *pico_socket_open(uint16_t net, uint16_t proto, void (*wakeup)(uint16_t ev, struct pico_socket *s))
{
    static struct pico_socket sock = {
        0
    };
    return &sock;
}

/* Used in dnsCallback */
int pico_socket_bind(struct pico_socket *s, void *local_addr, uint16_t *port)
{
    return 0;
}

/* Used in dnsCallback */
int pico_string_to_ipv4(const char *ipstr, uint32_t *ip)
{
    return 0;
}

/* Used in dnsCallback */
int pico_string_to_ipv6(const char *ipstr, uint8_t *ip)
{
    return 0;
}

/* Used in pico_sntp_client_wakeup */
int pico_socket_recvfrom(struct pico_socket *s, void *buf, int len, void *orig, uint16_t *remote_port)
{

    return 0;
}

/* Used in pico_sntp_send */
int pico_socket_sendto(struct pico_socket *s, const void *buf, int len, void *dst, uint16_t remote_port)
{
    return 0;
}

/* Used in pico_sntp_sync, not tested */
int pico_dns_client_getaddr(const char *url, void (*callback)(char *ip, void *arg), void *arg)
{
    return 0;
}

/* Used in pico_sntp_sync, not tested */
int pico_dns_client_getaddr6(const char *url, void (*callback)(char *, void *), void *arg)
{
    return 0;
}
/* Used in pico_sntp_parse */
void cb_synced(pico_err_t status)
{

}
struct pico_timer *pico_timer_add(pico_time expire, void (*timer)(pico_time, void *), void *arg)
{
    return NULL;
}

START_TEST(tc_timestamp_convert)
{
    struct pico_sntp_ts ts;
    struct pico_timeval tv;
    pico_time delay = 0ull;
    int ret = 0;

    /* Input is all zero */
    ts.sec = long_be(0ul);
    ts.frac = long_be(0ul);
    ret = timestamp_convert(&ts, &tv, delay);
    ck_assert(ret == -1);
    ck_assert(tv.tv_sec == 0);
    ck_assert(tv.tv_msec == 0);

    /* Minimum input*/
    ts.sec = long_be(SNTP_UNIX_OFFSET + 1390000000ul);
    ts.frac = long_be(4310344ul);     /* MIN value: 1msec */
    ret = timestamp_convert(&ts, &tv, delay);
    ck_assert(ret == 0);
    fail_unless(tv.tv_sec == 1390000000);
    fail_unless(tv.tv_msec == 1);

    /* Intermediate input */
    ts.sec = long_be(SNTP_UNIX_OFFSET + 1390000000ul);
    ts.frac = long_be(3865470566ul);    /* value: 899msec */
    ret = timestamp_convert(&ts, &tv, delay);
    ck_assert(ret == 0);
    fail_unless(tv.tv_sec == 1390000000);
    fail_unless(tv.tv_msec == 900);

    /* Maximum input */
    ts.sec = long_be(SNTP_UNIX_OFFSET + 1390000000ul);
    ts.frac = long_be(4294967295ul);    /* MAX value: 999msec */
    ret = timestamp_convert(&ts, &tv, delay);
    ck_assert(ret == 0);
    fail_unless(tv.tv_sec == 1390000001);
    fail_unless(tv.tv_msec == 0);

    /* Intermediate input with delay */
    ts.sec = long_be(SNTP_UNIX_OFFSET + 1390000000ul);
    ts.frac = long_be(3865470566ul);    /* value: 899msec */
    delay = 200ull;
    ret = timestamp_convert(&ts, &tv, delay);
    ck_assert(ret == 0);
    fail_unless(tv.tv_sec == 1390000001);
    fail_unless(tv.tv_msec == 100);
}
END_TEST
START_TEST(tc_pico_sntp_parse)
{
    /* TODO: test this: static void pico_sntp_parse(char *buf, struct sntp_server_ns_cookie *ck) */
    struct sntp_server_ns_cookie *ck; 
    struct pico_sntp_header header = {0};

    ck = malloc(sizeof(struct sntp_server_ns_cookie));
    fail_unless (ck);
    ck->hostname = malloc(sizeof(char)*5);
    fail_unless (ck->hostname);
    ck->stamp = 0ull;
    ck->cb_synced = cb_synced;

    header.mode = 4;    /* server mode */
    header.vn = 4;      /* sntp version 4 */
    header.stratum = 1; /* primary reference */
    header.trs_ts.sec = long_be(SNTP_UNIX_OFFSET + 1390000000ul);
    header.trs_ts.frac = long_be(3865470566ul);    /* value: 899msec */

    pico_sntp_parse((char *) &header, ck);
}
END_TEST
START_TEST(tc_pico_sntp_client_wakeup)
{
    /* TODO: test this: static void pico_sntp_client_wakeup(uint16_t ev, struct pico_socket *s) */
    uint16_t event = PICO_SOCK_EV_ERR;
    struct pico_socket sock = {
        0
    };
    struct sntp_server_ns_cookie *ck;
    ck = malloc(sizeof(struct sntp_server_ns_cookie));
    fail_unless (ck);
    sock.priv = ck;

    ck->cb_synced = cb_synced;
    printf("Started wakeup unit test\n");

    pico_sntp_client_wakeup(event, &sock);
}
END_TEST
START_TEST(tc_pico_sntp_send)
{
    /* TODO: test this: static void pico_sntp_send(struct pico_socket *sock, union pico_address *dst) */
    struct pico_socket sock = {
        0
    };
    union pico_address dst;
    struct sntp_server_ns_cookie ck = {
        0
    };
    sock.priv = &ck;

    pico_sntp_send(&sock, &dst);
}
END_TEST
START_TEST(tc_dnsCallback)
{
    /* TODO: test this: static void dnsCallback(char *ip, void *arg) */
    char ip[] = "198.123.30.132";
    struct sntp_server_ns_cookie *ck;
    ck = malloc(sizeof(struct sntp_server_ns_cookie));

    dnsCallback(ip, ck);
}
END_TEST


Suite *pico_suite(void)
{
    Suite *s = suite_create("PicoTCP");

    TCase *TCase_timestamp_convert = tcase_create("Unit test for pico_timeval");
    TCase *TCase_pico_sntp_send = tcase_create("Unit test for pico_sntp_send");
    TCase *TCase_pico_sntp_parse = tcase_create("Unit test for pico_sntp_parse");
    TCase *TCase_pico_sntp_client_wakeup = tcase_create("Unit test for pico_sntp_client_wakeup");
    TCase *TCase_dnsCallback = tcase_create("Unit test for dnsCallback");


    tcase_add_test(TCase_timestamp_convert, tc_timestamp_convert);
    suite_add_tcase(s, TCase_timestamp_convert);
    tcase_add_test(TCase_pico_sntp_send, tc_pico_sntp_send);
    suite_add_tcase(s, TCase_pico_sntp_send);
    tcase_add_test(TCase_pico_sntp_parse, tc_pico_sntp_parse);
    suite_add_tcase(s, TCase_pico_sntp_parse);
    tcase_add_test(TCase_pico_sntp_client_wakeup, tc_pico_sntp_client_wakeup);
    suite_add_tcase(s, TCase_pico_sntp_client_wakeup);
    tcase_add_test(TCase_dnsCallback, tc_dnsCallback);
    suite_add_tcase(s, TCase_dnsCallback);
    return s;
}

int main(void)
{
    int fails;
    Suite *s = pico_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fails = srunner_ntests_failed(sr);
    srunner_free(sr);
    return fails;
}
