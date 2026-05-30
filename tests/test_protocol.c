#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"
#include "gen_dlist.h"
#include "client_groups_mng.h"

void test_protocol_pack_unpack_str() {
    uint8_t buf[TLV_MAX_PAYLOAD];
    char out[64];
    const char *str = "hello world";
    size_t len = Proto_PackStr(buf, str);
    assert(len == strlen(str) + 1);
    assert(Proto_UnpackStr(buf, len, out, sizeof(out)) == 0);
    assert(strcmp(str, out) == 0);
}

void test_protocol_pack_unpack_userpass() {
    uint8_t buf[TLV_MAX_PAYLOAD];
    char user[32], pass[32];
    const char *u = "alice", *p = "secret";
    size_t len = Proto_PackUserPass(buf, u, p);
    assert(Proto_UnpackUserPass(buf, len, user, sizeof(user), pass, sizeof(pass)) == 0);
    assert(strcmp(u, user) == 0);
    assert(strcmp(p, pass) == 0);
}

void test_protocol_pack_unpack_groupresp() {
    uint8_t buf[TLV_MAX_PAYLOAD];
    char addr[32];
    uint16_t port = 12345, out_port;
    uint8_t status, out_status;
    status = SUCCESS;
    size_t len = Proto_PackGroupResp(buf, status, "224.0.0.1", port);
    assert(Proto_UnpackGroupResp(buf, len, &out_status, addr, sizeof(addr), &out_port) == 0);
    assert(out_status == SUCCESS);
    assert(strcmp(addr, "224.0.0.1") == 0);
    assert(out_port == port);
}

void test_gen_dlist() {
    List *list = ListCreate();
    assert(list);
    int a = 1, b = 2, c = 3;
    ListPushTail(list, &a);
    ListPushTail(list, &b);
    ListPushTail(list, &c);
    assert(ListSize(list) == 3);
    assert(*(int*)ListPopHead(list) == 1);
    assert(ListSize(list) == 2);
    ListDestroy(&list, NULL);
    assert(list == NULL);
}

void test_client_groups_mng() {
    GroupInfo g1 = {0};
    strcpy(g1.name, "group1");
    strcpy(g1.multicast_ip, "224.0.0.1");
    g1.port = 1234;
    g1.receiver_pid = 111;
    g1.sender_pid = 222;
    add_group(&g1);
    GroupInfo *found = get_group("group1");
    assert(found && strcmp(found->name, "group1") == 0);
    remove_group("group1");
    assert(get_group("group1") == NULL);
    groups_cleanup();
}

int main() {
    printf("Running protocol tests...\n");
    test_protocol_pack_unpack_str();
    test_protocol_pack_unpack_userpass();
    test_protocol_pack_unpack_groupresp();
    printf("Protocol tests passed.\n");

    printf("Running gen_dlist tests...\n");
    test_gen_dlist();
    printf("gen_dlist tests passed.\n");

    printf("Running client_groups_mng tests...\n");
    test_client_groups_mng();
    printf("client_groups_mng tests passed.\n");

    printf("All tests passed!\n");
    return 0;
}
