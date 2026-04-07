#include "products.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/server/handlers/exceptions.hpp>

ProductsUpdateHandler::ProductsUpdateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) : {

    pg_cluster_ =
        context.FindComponent<userver::storages::postgres::Component>()
            .GetCluster();
}

userver::formats::json::Value ProductsUpdateHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    int id = std::stoi(request.GetPathArg("id"));
    auto body = userver::formats::json::FromString(request.RequestBody());

    auto name = body["name"].As<std::string>();
    auto sku = body["sku"].As<std::string>();
    auto price = body["price"].As<double>();

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "UPDATE products SET name=$1, sku=$2, price=$3 WHERE id=$4",
        name, sku, price, id);

    if (result.RowsAffected() == 0) {
        throw userver::server::handlers::ResourceNotFound(
            userver::server::handlers::ExternalBody{"Product not found"});
    }

    userver::formats::json::ValueBuilder response;
    response["id"] = id;
    response["status"] = "updated";

    return response.ExtractValue();
}