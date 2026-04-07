#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/storages/postgres/component.hpp>

#include "handlers/auth.hpp"
#include "handlers/products.hpp"
#include "handlers/inventory.hpp"
#include "handlers/users.hpp"
#include "handlers/arrivals.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::storages::postgres::Component>("postgres-db")
            .Append<AuthHandler>()
            .Append<ProductsHandler>()
            .Append<ProductsCreateHandler>()
            .Append<ProductsUpdateHandler>()
            .Append<ProductsDeleteHandler>()
            .Append<ProductsSearchHandler>()
            .Append<InventoryHandler>()
            .Append<InventoryWriteoffHandler>()
            .Append<UserHandler>()
            .Append<ArrivalsHandlerGet>()
            .Append<ArrivalsHandlerPost>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}