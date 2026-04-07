#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

// GET /inventory
class InventoryHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-inventory";

    InventoryHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext& context) const override {

        auto& pg_component = context.FindComponent<userver::components::Postgres>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
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
        return userver::formats::json::ToString(items.ExtractValue());
    }
};

// POST /inventory/writeoff
class InventoryWriteoffHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-inventory-writeoff";

    InventoryWriteoffHandler(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        auto body_json = userver::formats::json::FromString(request.RequestBody());
        int product_id = body_json["product_id"].As<int>();
        int quantity = body_json["quantity"].As<int>();
        std::string reason = body_json["reason"].As<std::string>("unknown");

        if (quantity <= 0) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"Quantity must be positive"});
        }

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto transaction = cluster->Begin({});  // пустые опции транзакции

        // Проверяем остаток
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

        // Обновляем остаток
        transaction.Execute(
            "UPDATE inventory SET quantity = quantity - $1, last_updated = NOW() WHERE product_id = $2",
            quantity, product_id);
        // Записываем списание
        transaction.Execute(
            "INSERT INTO writeoffs (product_id, quantity, reason) VALUES ($1, $2, $3)",
            product_id, quantity, reason);
        transaction.Commit();

        userver::formats::json::ValueBuilder response;
        response["status"] = "writeoff processed";
        response["product_id"] = product_id;
        response["quantity_written_off"] = quantity;
        return userver::formats::json::ToString(response.ExtractValue());
    }
};