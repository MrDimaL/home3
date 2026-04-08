#include "inventory.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/server/handlers/exceptions.hpp>

InventoryWriteoffHandler::InventoryWriteoffHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
        context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value InventoryWriteoffHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {

    const auto& body = request_json;

    if (!body.HasMember("product_id") || !body.HasMember("quantity")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required fields"});
    }

    int product_id = body["product_id"].As<int>();
    int quantity = body["quantity"].As<int>();
    std::string reason = body["reason"].As<std::string>("unknown");

    if (quantity <= 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Quantity must be positive"});
    }

    auto transaction = pg_cluster_->Begin({});

    auto check = transaction.Execute(
        "SELECT quantity FROM inventory WHERE product_id = $1 FOR UPDATE",
        product_id);

    if (check.IsEmpty()) {
        throw userver::server::handlers::ResourceNotFound(
            userver::server::handlers::ExternalBody{"Product not found in inventory"});
    }

    int current_qty = check[0][0].As<int>();

    if (current_qty < quantity) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Not enough stock"});
    }

    transaction.Execute(
        "UPDATE inventory SET quantity = quantity - $1, last_updated = NOW() WHERE product_id = $2",
        quantity, product_id);

    transaction.Execute(
        "INSERT INTO writeoffs (product_id, quantity, reason) VALUES ($1, $2, $3)",
        product_id, quantity, reason);

    transaction.Commit();

    userver::formats::json::ValueBuilder response;
    response["status"] = "writeoff processed";
    response["product_id"] = product_id;
    response["quantity_written_off"] = quantity;

    return response.ExtractValue();
}
