#include "products.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/server/handlers/exceptions.hpp>

ProductsDeleteHandler::ProductsDeleteHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) : {

    pg_cluster_ =
        context.FindComponent<userver::storages::postgres::Component>()
            .GetCluster();
}

userver::formats::json::Value ProductsDeleteHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    int id = std::stoi(request.GetPathArg("id"));

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "DELETE FROM products WHERE id = $1",
        id);

    if (result.RowsAffected() == 0) {
        throw userver::server::handlers::ResourceNotFound(
            userver::server::handlers::ExternalBody{"Product not found"});
    }

    userver::formats::json::ValueBuilder response;
    response["id"] = id;
    response["status"] = "deleted";

    return response.ExtractValue();
}