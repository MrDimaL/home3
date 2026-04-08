#include "arrivals.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/logging/log.hpp>

ArrivalsHandlerGet::ArrivalsHandlerGet(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
        context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value ArrivalsHandlerGet::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    try {
        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            R"(
            SELECT 
                a.id,
                COALESCE(p.name, 'unknown') as product_name,
                a.quantity,
                a.arrival_date::text as arrival_date,
                COALESCE(u.username, 'system') as created_by 
            FROM arrivals a 
            LEFT JOIN products p ON a.product_id = p.id
            LEFT JOIN users u ON a.created_by = u.id
            ORDER BY a.arrival_date DESC
            )");

        userver::formats::json::ValueBuilder items;

        for (const auto& row : result) {
            userver::formats::json::ValueBuilder item;

            item["id"] = row["id"].As<int>();
            item["product"] = row["product_name"].As<std::string>();
            item["quantity"] = row["quantity"].As<int>();

            item["date"] = row["arrival_date"].As<std::string>();

            item["created_by"] = row["created_by"].As<std::string>();

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
