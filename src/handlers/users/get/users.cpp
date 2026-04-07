#include "users.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/server/handlers/exceptions.hpp>

UserHandler::UserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
        context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value UserHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    auto username = request.GetPathArg("username");

    if (username.empty()) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Username required"});
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        "SELECT username, full_name, role FROM users WHERE username = $1",
        username);

    if (result.IsEmpty()) {
        throw userver::server::handlers::ResourceNotFound(
            userver::server::handlers::ExternalBody{"User not found"});
    }

    const auto& row = result[0];

    userver::formats::json::ValueBuilder response;
    response["username"] = row["username"].As<std::string>();
    response["full_name"] = row["full_name"].As<std::string>();
    response["role"] = row["role"].As<std::string>();

    return response.ExtractValue();
}
