#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

class UserHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-user";

    UserHandler(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        auto username = request.GetPathArg("username");
        if (username.empty()) {
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{"Username required"});
        }

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            "SELECT username, full_name, role FROM users WHERE username = $1",
            username);

        if (result.IsEmpty()) {
            throw userver::server::handlers::ResourceNotFound(
                userver::server::handlers::ExternalBody{"User not found"});
        }

        auto row = result[0];
        userver::formats::json::ValueBuilder response;
        response["username"] = row["username"].As<std::string>();
        response["full_name"] = row["full_name"].As<std::string>();
        response["role"] = row["role"].As<std::string>();

        return userver::formats::json::ToString(response.ExtractValue());
    }
};