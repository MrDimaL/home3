#include "products.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>

ProductsHandler::ProductsHandler(
    const userver::components::ComponentConfig &config,
    const userver::components::ComponentContext &context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) : {
    
    pg_cluster_ =
      context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value ProductsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest &request,
    const formats::json::Value &request_json,
    userver::server::request::RequestContext &request_context) {
  auto result =
      cluster->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                       "SELECT id, name, sku, price FROM products ORDER BY id");

  userver::formats::json::ValueBuilder items;
  for (const auto &row : result) {
    userver::formats::json::ValueBuilder item;
    item["id"] = row["id"].As<int>();
    item["name"] = row["name"].As<std::string>();
    item["sku"] = row["sku"].As<std::string>();
    item["price"] = row["price"].As<double>();
    items.PushBack(std::move(item));
  }

  return items.ExtractValue();
}
