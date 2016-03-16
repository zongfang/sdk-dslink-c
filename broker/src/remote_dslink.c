#include <string.h>
#include <dslink/utils.h>
#include <broker/permission/permission.h>
#include "broker/stream.h"

int broker_remote_dslink_init(RemoteDSLink *link) {
    memset(link, 0, sizeof(RemoteDSLink));
    int ret = 0;
    if (dslink_map_init(&link->responder_streams, dslink_map_uint32_cmp,
                           dslink_map_uint32_key_len_cal, dslink_map_hash_key) != 0) {
        return ret;
    }
    if (dslink_map_init(&link->requester_streams, dslink_map_uint32_cmp,
                           dslink_map_uint32_key_len_cal, dslink_map_hash_key) != 0) {
        dslink_map_free(&link->responder_streams);
        return ret;
    }
    permission_groups_init(&link->permission_groups);
    return ret;
}

void broker_remote_dslink_free(RemoteDSLink *link) {
    if (link->auth) {
        mbedtls_ecdh_free(&link->auth->tempKey);
        DSLINK_CHECKED_EXEC(free, (void *) link->auth->pubKey);
        dslink_free(link->auth);
    }

    dslink_map_foreach(&link->requester_streams) {
        BrokerStream *stream = entry->value->data;
        requester_stream_closed(stream, link);
        entry->value->data = NULL;
    }
    dslink_map_free(&link->requester_streams);

    dslink_map_foreach(&link->responder_streams) {
        BrokerStream *stream = entry->value->data;
        responder_stream_closed(stream, link);
        // free the node only when resp_close_callback return TRUE
        entry->value->data = NULL;
    }
    dslink_map_free(&link->responder_streams);
    permission_groups_free(&link->permission_groups);
    dslink_free((void *) link->path);
    json_decref(link->linkData);
    wslay_event_context_free(link->ws);
    link->ws = NULL;
}
