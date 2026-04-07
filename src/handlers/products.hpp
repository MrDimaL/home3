#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

// GET /products
class ProductsHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-products";

    ProductsHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext& context) const override {

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            "SELECT id, name, sku, price FROM products ORDER BY id");

        userver::formats::json::ValueBuilder items;
        for (const auto& row : result) {
            userver::formats::json::ValueBuilder item;
            item["id"] = row["id"].As<int>();
            item["name"] = row["name"].As<std::string>();
            item["sku"] = row["sku"].As<std::string>();
            item["price"] = row["price"].As<double>();
            items.PushBack(std::move(item));
        }
        return userver::formats::json::ToString(items.ExtractValue());
    }
};

// POST /products
class ProductsCreateHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-products-create";

    ProductsCreateHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        auto body_json = userver::formats::json::FromString(request.RequestBody());
        auto name = body_json["name"].As<std::string>();
        auto sku = body_json["sku"].As<std::string>();
        auto price = body_json["price"].As<double>();

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO products (name, sku, price) VALUES ($1, $2, $3) RETURNING id",
            name, sku, price);
        int new_id = result[0][0].As<int>();

        // Создаём запись в инвентаре
        cluster->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO inventory (product_id, quantity) VALUES ($1, 0)",
            new_id);

        userver::formats::json::ValueBuilder response;
        response["id"] = new_id;
        response["status"] = "created";
        return userver::formats::json::ToString(response.ExtractValue());
    }
};

// PATCH /products/{id}
class ProductsUpdateHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-products-update";

    ProductsUpdateHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        int id = std::stoi(request.GetPathArg("id"));
        auto body_json = userver::formats::json::FromString(request.RequestBody());
        auto name = body_json["name"].As<std::string>();
        auto sku = body_json["sku"].As<std::string>();
        auto price = body_json["price"].As<double>();

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "UPDATE products SET name = $1, sku = $2, price = $3 WHERE id = $4",
            name, sku, price, id);

        if (result.RowsAffected() == 0) {
            throw userver::server::handlers::ResourceNotFound(
                userver::server::handlers::ExternalBody{"Product not found"});
        }

        userver::formats::json::ValueBuilder response;
        response["id"] = id;
        response["status"] = "updated";
        return userver::formats::json::ToString(response.ExtractValue());
    }
};

// DELETE /products/{id}
class ProductsDeleteHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-products-delete";

    ProductsDeleteHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        int id = std::stoi(request.GetPathArg("id"));

        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
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
        return userver::formats::json::ToString(response.ExtractValue());
    }
};

// GET /products/search?q=...
class ProductsSearchHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-products-search";

    ProductsSearchHandler(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context) const override {

        auto query = request.GetArg("q");
        if (query.empty()) return "[]";

        auto search_pattern = "%" + query + "%";
        auto& pg_component = context.FindComponent<userver::storages::postgres::Component>();
        auto cluster = pg_component.GetCluster();

        auto result = cluster->Execute(
            userver::storages::postgres::ClusterHostType::kSlave,
            "SELECT id, name, sku, price FROM products WHERE name ILIKE $1 OR sku ILIKE $1 ORDER BY name LIMIT 50",
            search_pattern);

        userver::formats::json::ValueBuilder items;
        for (const auto& row : result) {
            userver::formats::json::ValueBuilder item;
            item["id"] = row["id"].As<int>();
            item["name"] = row["name"].As<std::string>();
            item["sku"] = row["sku"].As<std::string>();
            item["price"] = row["price"].As<double>();
            items.PushBack(std::move(item));
        }
        return userver::formats::json::ToString(items.ExtractValue());
    }
};