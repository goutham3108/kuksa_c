#include "kuksa_client.h"

#include <grpc/grpc.h>
#include <grpc/byte_buffer.h>
#include <grpc/grpc_security.h>
#include <grpc/support/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Generated from kuksa-databroker proto files with protobuf-c + grpc-c plugin.
 * See scripts/generate_proto.sh
 */
#include "kuksa/val/v1/types.pb-c.h"
#include "kuksa/val/v1/val.pb-c.h"

static int call_unary(
    const char *target,
    const char *method,
    ProtobufCMessage *request,
    const ProtobufCMessageDescriptor *response_descriptor,
    ProtobufCMessage **response_out,
    const char *token) {
  int rc = 1;
  grpc_channel *channel = NULL;
  grpc_call *call = NULL;
  grpc_slice method_slice = grpc_slice_from_copied_string(method);
  grpc_slice host_slice = grpc_slice_from_static_string("");
  grpc_metadata_array initial_metadata_recv;
  grpc_metadata_array trailing_metadata_recv;
  grpc_byte_buffer *response_payload_recv = NULL;
  grpc_status_code status;
  grpc_slice status_details;
  gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(5, GPR_TIMESPAN));

  grpc_init();
  grpc_metadata_array_init(&initial_metadata_recv);
  grpc_metadata_array_init(&trailing_metadata_recv);

  grpc_channel_credentials *creds = grpc_insecure_credentials_create();
  channel = grpc_channel_create(target, creds, NULL);
  grpc_channel_credentials_release(creds);

  if (!channel) {
    fprintf(stderr, "failed to create channel\n");
    goto cleanup;
  }

  call = grpc_channel_create_call(channel, NULL, GRPC_PROPAGATE_DEFAULTS, NULL, method_slice, &host_slice, deadline, NULL);
  if (!call) {
    fprintf(stderr, "failed to create call\n");
    goto cleanup;
  }

  size_t packed_size = protobuf_c_message_get_packed_size(request);
  uint8_t *packed = malloc(packed_size);
  if (!packed) {
    fprintf(stderr, "allocation failed\n");
    goto cleanup;
  }
  protobuf_c_message_pack(request, packed);

  grpc_slice request_slice = grpc_slice_from_copied_buffer((const char *) packed, packed_size);
  grpc_byte_buffer *request_payload = grpc_raw_byte_buffer_create(&request_slice, 1);

  grpc_metadata metadata[1];
  size_t metadata_count = 0;
  if (token && token[0]) {
    metadata[0].key = "authorization";
    size_t token_len = strlen(token) + strlen("Bearer ") + 1;
    char *auth = malloc(token_len);
    snprintf(auth, token_len, "Bearer %s", token);
    metadata[0].value = auth;
    metadata[0].value_length = strlen(auth);
    metadata[0].flags = 0;
    metadata_count = 1;
  }

  grpc_op ops[6];
  memset(ops, 0, sizeof(ops));
  ops[0].op = GRPC_OP_SEND_INITIAL_METADATA;
  ops[0].data.send_initial_metadata.count = metadata_count;
  ops[0].data.send_initial_metadata.metadata = metadata_count ? metadata : NULL;

  ops[1].op = GRPC_OP_SEND_MESSAGE;
  ops[1].data.send_message.send_message = request_payload;

  ops[2].op = GRPC_OP_SEND_CLOSE_FROM_CLIENT;

  ops[3].op = GRPC_OP_RECV_INITIAL_METADATA;
  ops[3].data.recv_initial_metadata.recv_initial_metadata = &initial_metadata_recv;

  ops[4].op = GRPC_OP_RECV_MESSAGE;
  ops[4].data.recv_message.recv_message = &response_payload_recv;

  ops[5].op = GRPC_OP_RECV_STATUS_ON_CLIENT;
  ops[5].data.recv_status_on_client.trailing_metadata = &trailing_metadata_recv;
  ops[5].data.recv_status_on_client.status = &status;
  ops[5].data.recv_status_on_client.status_details = &status_details;

  grpc_completion_queue *cq = grpc_completion_queue_create_for_next(NULL);
  grpc_call_error call_error = grpc_call_start_batch(call, ops, 6, (void *) 1, NULL);
  if (call_error != GRPC_CALL_OK) {
    fprintf(stderr, "grpc_call_start_batch failed: %d\n", call_error);
    grpc_completion_queue_destroy(cq);
    grpc_byte_buffer_destroy(request_payload);
    grpc_slice_unref(request_slice);
    free(packed);
    goto cleanup;
  }

  grpc_event event = grpc_completion_queue_next(cq, deadline, NULL);
  grpc_completion_queue_shutdown(cq);
  grpc_completion_queue_destroy(cq);

  grpc_byte_buffer_destroy(request_payload);
  grpc_slice_unref(request_slice);
  free(packed);

  if (metadata_count) {
    free((void *) metadata[0].value);
  }

  if (event.type != GRPC_OP_COMPLETE || !event.success) {
    fprintf(stderr, "grpc call failed to complete\n");
    goto cleanup;
  }

  if (status != GRPC_STATUS_OK) {
    fprintf(stderr, "grpc status: %d (%.*s)\n", status, (int) GRPC_SLICE_LENGTH(status_details), GRPC_SLICE_START_PTR(status_details));
    grpc_slice_unref(status_details);
    goto cleanup;
  }

  if (!response_payload_recv) {
    fprintf(stderr, "empty response payload\n");
    goto cleanup;
  }

  grpc_byte_buffer_reader reader;
  grpc_byte_buffer_reader_init(&reader, response_payload_recv);
  grpc_slice response_slice = grpc_byte_buffer_reader_readall(&reader);

  *response_out = protobuf_c_message_unpack(
      response_descriptor,
      NULL,
      GRPC_SLICE_LENGTH(response_slice),
      GRPC_SLICE_START_PTR(response_slice));

  grpc_slice_unref(response_slice);
  grpc_slice_unref(status_details);

  if (!*response_out) {
    fprintf(stderr, "failed to unpack response\n");
    goto cleanup;
  }

  rc = 0;

cleanup:
  if (response_payload_recv) grpc_byte_buffer_destroy(response_payload_recv);
  grpc_metadata_array_destroy(&initial_metadata_recv);
  grpc_metadata_array_destroy(&trailing_metadata_recv);
  if (call) grpc_call_unref(call);
  if (channel) grpc_channel_destroy(channel);
  grpc_slice_unref(method_slice);
  grpc_shutdown();
  return rc;
}

int kuksa_get_current_value(const char *target, const char *token, const char *path) {
  Kuksa__Val__V1__GetRequest req = KUKSA__VAL__V1__GET_REQUEST__INIT;
  Kuksa__Val__V1__EntryRequest entry = KUKSA__VAL__V1__ENTRY_REQUEST__INIT;

  entry.path = (char *) path;
  req.entries = &entry;
  req.n_entries = 1;

  ProtobufCMessage *response = NULL;
  int rc = call_unary(
      target,
      "/kuksa.val.v1.VAL/Get",
      &req.base,
      &kuksa__val__v1__get_response__descriptor,
      &response,
      token);

  if (rc != 0) return rc;

  Kuksa__Val__V1__GetResponse *get_response = (Kuksa__Val__V1__GetResponse *) response;
  if (get_response->n_entries > 0 && get_response->entries[0]->value && get_response->entries[0]->value->string) {
    printf("%s = %s\n", path, get_response->entries[0]->value->string);
  } else {
    printf("%s = <no string value in response>\n", path);
  }

  protobuf_c_message_free_unpacked(response, NULL);
  return 0;
}

int kuksa_set_current_value(const char *target, const char *token, const char *path, const char *value) {
  Kuksa__Val__V1__SetRequest req = KUKSA__VAL__V1__SET_REQUEST__INIT;
  Kuksa__Val__V1__DataEntry entry = KUKSA__VAL__V1__DATA_ENTRY__INIT;
  Kuksa__Val__V1__Datapoint dp = KUKSA__VAL__V1__DATAPOINT__INIT;

  entry.path = (char *) path;
  entry.value = &dp;
  dp.string = (char *) value;

  req.entries = &entry;
  req.n_entries = 1;

  ProtobufCMessage *response = NULL;
  int rc = call_unary(
      target,
      "/kuksa.val.v1.VAL/Set",
      &req.base,
      &kuksa__val__v1__set_response__descriptor,
      &response,
      token);

  if (rc == 0) {
    printf("set %s = %s\n", path, value);
  }

  if (response) protobuf_c_message_free_unpacked(response, NULL);
  return rc;
}
