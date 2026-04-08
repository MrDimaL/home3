#include "products.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/decimal64/decimal64.hpp>
#include <userver/storages/postgres/io/decimal64.hpp>
#include <userver/logging/log.hpp>

ProductsHandler::ProductsHandler(
    const userver::components::ComponentConfig &config,
    const userver::components::ComponentContext &context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
      context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value ProductsHandler::HandleRequestJsonThrow(
  const userver::server::http::HttpRequest &request,
  const userver::formats::json::Value &request_json,
  userver::server::request::RequestContext &request_context) const {
  
  try {
    auto result =
      pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                       "SELECT id, name, sku, price FROM products ORDER BY id");

    userver::formats::json::ValueBuilder items;

    for (const auto &row : result) {
      userver::formats::json::ValueBuilder item;
      item["id"] = row["id"].As<int>();
      item["name"] = row["name"].As<std::string>();
      item["sku"] = row["sku"].As<std::string>();

      auto price = row["price"].As<userver::decimal64::Decimal<2>>();
      item["price"] = std::stod(userver::decimal64::ToString(price));

      items.PushBack(std::move(item));
    }

    return items.ExtractValue();
    
  } catch (const std::exception& e) {
        LOG_ERROR() << "ARRIVALS ERROR: " << e.what();

        userver::formats::json::ValueBuilder err;
        err["error"] = e.what();

        throw userver::server::handlers::InternalServerError(
            userver::server::handlers::ExternalBody{userver::formats::json::ToString(err.ExtractValue())});
  }
}
