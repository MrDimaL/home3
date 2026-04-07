#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

// GET /arrivals
class ArrivalsHandlerGet final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-arrivals-get";

    ArrivalsHandlerGet(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext& context) const override {

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            "SELECT a.id, p.name as product_name, a.quantity, a.arrival_date, u.username as created_by "
            "FROM arrivals a "
            "JOIN products p ON a.product_id = p.id "
            "LEFT JOIN users u ON a.created_by = u.id "
            "ORDER BY a.arrival_date DESC");

        userver::formats::json::ValueBuilder items;
        for (const auto& row : result) {
            userver::formats::json::ValueBuilder item;
            item["id"] = row["id"].As<int>();
            item["product"] = row["product_name"].As<std::string>();
            item["quantity"] = row["quantity"].As<int>();
            item["date"] = row["arrival_date"].As<std::string>();
            if (!row["created_by"].IsNull()) {
                item["created_by"] = row["created_by"].As<std::string>();
            }
            items.PushBack(std::move(item));
        }
        return userver::formats::json::ToString(items.ExtractValue());
    }
};

// POST /arrivals
class ArrivalsHandlerPost final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-arrivals-post";

    ArrivalsHandlerPost(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        auto body_json = userver::formats::json::FromString(request.RequestBody());
        int product_id = body_json["product_id"].As<int>();
        int quantity = body_json["quantity"].As<int>();
        std::string date_str = body_json["date"].As<std::string>();
        int created_by = 1; // demo

        if (quantity <= 0) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"Quantity must be positive"});
        }

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto transaction = cluster->Begin({});

        auto insert_res = transaction.Execute(
            "INSERT INTO arrivals (product_id, quantity, arrival_date, created_by) "
            "VALUES ($1, $2, $3, $4) RETURNING id",
            product_id, quantity, date_str, created_by);
        int arrival_id = insert_res[0][0].As<int>();

        auto update_res = transaction.Execute(
            "UPDATE inventory SET quantity = quantity + $1, last_updated = NOW() "
            "WHERE product_id = $2",
            quantity, product_id);
        if (update_res.RowsAffected() == 0) {
            transaction.Execute(
                "INSERT INTO inventory (product_id, quantity) VALUES ($1, $2)",
                product_id, quantity);
        }
        transaction.Commit();

        userver::formats::json::ValueBuilder response;
        response["status"] = "created";
        response["arrival_id"] = arrival_id;
        return userver::formats::json::ToString(response.ExtractValue());
    }
};