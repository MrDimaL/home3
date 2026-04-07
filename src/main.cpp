#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/postgres/component.hpp>

#include "handlers/auth/post/auth.hpp"
#include <handlers/arrivals/post/arrivals.hpp>
#include <handlers/arrivals/get/arrivals.hpp>
#include <handlers/inventory/post/inventory.hpp>
#include <handlers/inventory/get/inventory.hpp>
#include <handlers/products/post/products.hpp>
#include <handlers/products/get/products.hpp>
#include <handlers/products/delete/products.hpp>
#include <handlers/products/patch/products.hpp>
#include <handlers/users/get/users.hpp>

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::components::Postgres>("postgres-db")
            .Append<AuthHandler>()
            .Append<ProductsHandler>()
            .Append<ProductsCreateHandler>()
            .Append<ProductsUpdateHandler>()
            .Append<ProductsDeleteHandler>()
            // .Append<ProductsSearchHandler>() TODO: не нашёл нужный handler
            .Append<InventoryHandler>()
            .Append<InventoryWriteoffHandler>()
            .Append<UserHandler>()
            .Append<ArrivalsHandlerGet>()
            .Append<ArrivalsHandlerPost>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
