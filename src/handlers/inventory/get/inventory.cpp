#include "inventory.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>

InventoryHandler::InventoryHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) : {

    pg_cluster_ =
        context.FindComponent<userver::storages::postgres::Component>()
            .GetCluster();
}

userver::formats::json::Value InventoryHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT p.id, p.name, p.sku, i.quantity, i.last_updated "
        "FROM inventory i JOIN products p ON i.product_id = p.id "
        "ORDER BY p.id");

    userver::formats::json::ValueBuilder items;

    for (const auto& row : result) {
        userver::formats::json::ValueBuilder item;
        item["product_id"] = row["id"].As<int>();
        item["product_name"] = row["name"].As<std::string>();
        item["sku"] = row["sku"].As<std::string>();
        item["quantity"] = row["quantity"].As<int>();
        item["last_updated"] = row["last_updated"].As<std::string>();
        items.PushBack(std::move(item));
    }

    return items.ExtractValue();
}