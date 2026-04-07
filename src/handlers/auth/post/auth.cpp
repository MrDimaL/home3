#include "auth.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>


AuthHandler::AuthHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
        context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value AuthHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT EXISTS(SELECT 1 FROM users WHERE username = 'admin')");

    bool exists = result[0][0].As<bool>();

    userver::formats::json::ValueBuilder response;
    response["token"] = exists ? "demo-token-123" : "invalid";
    response["success"] = exists;

    return response.ExtractValue();
}
