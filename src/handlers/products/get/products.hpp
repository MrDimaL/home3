#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/components/component_context.hpp>


class ProductsHandler final
    : public userver::server::handlers::HttpHandlerJsonBase {
public:
  static constexpr std::string_view kName = "handler-products";

  ProductsHandler(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest &, const userver::formats::json::Value &,
      userver::server::request::RequestContext &) const override;

private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};
