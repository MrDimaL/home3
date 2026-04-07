#include "products.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>

ProductsCreateHandler::ProductsCreateHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) : {

    pg_cluster_ =
        context.FindComponent<userver::storages::postgres::Component>()
            .GetCluster();
}

userver::formats::json::Value ProductsCreateHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

    auto body = userver::formats::json::FromString(request.RequestBody());

    auto name = body["name"].As<std::string>();
    auto sku = body["sku"].As<std::string>();
    auto price = body["price"].As<double>();

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO products (name, sku, price) VALUES ($1, $2, $3) RETURNING id",
        name, sku, price);

    int new_id = result[0][0].As<int>();

    pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO inventory (product_id, quantity) VALUES ($1, 0)",
        new_id);

    userver::formats::json::ValueBuilder response;
    response["id"] = new_id;
    response["status"] = "created";

    return response.ExtractValue();
}