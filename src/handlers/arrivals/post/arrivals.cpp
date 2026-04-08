#include "arrivals.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/components/component_context.hpp>


ArrivalsHandlerPost::ArrivalsHandlerPost(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {

    pg_cluster_ =
        context.FindComponent<userver::components::Postgres>("postgres-db")
            .GetCluster();
}

userver::formats::json::Value ArrivalsHandlerPost::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {

    const auto& body = request_json;

    if (!body.HasMember("product_id") || !body.HasMember("quantity") || !body.HasMember("date")) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Missing required fields"});
    }

    int product_id = body["product_id"].As<int>();
    int quantity = body["quantity"].As<int>();
    std::string date_str = body["date"].As<std::string>();
    int created_by = 1;

    if (quantity <= 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::ExternalBody{"Quantity must be positive"});
    }

    auto transaction = pg_cluster_->Begin({});

    auto insert_res = transaction.Execute(
        "INSERT INTO arrivals (product_id, quantity, arrival_date, created_by) "
        "VALUES ($1, $2, $3, $4) RETURNING id",
        product_id, quantity, date_str, created_by);

    int arrival_id = insert_res[0][0].As<int>();

    auto update_res = transaction.Execute(
        "UPDATE inventory SET quantity = quantity + $1 WHERE product_id = $2",
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

    return response.ExtractValue();
}
