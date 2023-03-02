// server receives a schema
#include <capnp/rpc-twoparty.h>
#include <kj/async-io.h>
#include <capnp/dynamic.h>
#include <capnp/message.h>
#include <capnp/schema-loader.h>
#include <capnp/schema.h>
// #include <kj/async-unix.h>
#include <iostream>

#include "generic.capnp.h"

class Loader_impl final : public InterfaceLoader::Server
{
public:
    kj::Promise<void> load(LoadContext context) override
    {
        std::cout << "load context\n";
        auto message = context.getParams();
        auto user_schema = message.getSchema();

        auto schema_loader = capnp::SchemaLoader();

        for (auto schema : user_schema.getSchemas())
        {
            schema_loader.load(schema);
        }

        capnp::StructSchema schema = schema_loader.get(user_schema.getTypeId()).asStruct();
        // auto loaded_schema = schema_loader.load(schema_);

        // how to do below without malloc?

        // cannot init result as a DynamicStruct
        auto mb = capnp::MallocMessageBuilder();
        auto msg = mb.initRoot<capnp::DynamicStruct>(schema);
        msg.set("text", "abc");
        auto timestamp = msg.init("timestamp").as<capnp::DynamicStruct>();
        timestamp.set("nanoseconds", 123);

        auto results = context.initResults();
        results.setValue(msg.asReader());

        return kj::READY_NOW;
    }
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: ";
        auto path = argv[0];
        std::cout << path << " hostname:port" << std::endl;
        return 0;
    }

    auto bind_address = argv[1];
    auto io_context = kj::setupAsyncIo();

    auto &wait_scope = io_context.waitScope;
    auto listener = io_context.provider->getNetwork().parseAddress(bind_address).wait(wait_scope)->listen();

    auto &&server = capnp::TwoPartyServer{kj::heap<Loader_impl>()};
    auto serv = server.listen(*listener);
    serv.wait(wait_scope);
}
