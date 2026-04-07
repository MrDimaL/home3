#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

class AuthHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-auth";

    AuthHandler(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext& context) const override {

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            "SELECT EXISTS(SELECT 1 FROM users WHERE username = 'admin')"
        );
        bool exists = result[0][0].As<bool>();

        userver::formats::json::ValueBuilder response;
        response["token"] = exists ? "demo-token-123" : "invalid";
        response["success"] = exists;

        return userver::formats::json::ToString(response.ExtractValue());
    }
};