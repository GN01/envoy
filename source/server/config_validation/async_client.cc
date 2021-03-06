#include "server/config_validation/async_client.h"

namespace Envoy {
namespace Http {

ValidationAsyncClient::ValidationAsyncClient(Api::Api& api) : dispatcher_(api) {}

AsyncClient::Request* ValidationAsyncClient::send(MessagePtr&&, Callbacks&, const RequestOptions&) {
  return nullptr;
}

AsyncClient::Stream* ValidationAsyncClient::start(StreamCallbacks&, const StreamOptions&) {
  return nullptr;
}

} // namespace Http
} // namespace Envoy
