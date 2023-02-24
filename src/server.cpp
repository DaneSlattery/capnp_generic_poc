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

class Loader_impl final : public GenericInterface::InterfaceLoader::Server
{
public:
    Loader_impl(GenericInterface::ClientInterface::Client client) : client{std::move(client)}
    {
    }
    kj::Promise<void> load(LoadContext context) override
    {
        std::cout << "load context\n";
        auto message = context.getParams();
        auto struct_ = message.getStruct();
        auto schema_loader = capnp::SchemaLoader();
        // schema_loader.loadCompiledTypeAndDependencies<Message>();
        auto loaded_schema = schema_loader.load(struct_.getUserSchema());

        auto mb = capnp::MallocMessageBuilder();
        auto msg = mb.initRoot<capnp::DynamicStruct>(loaded_schema.asStruct());
        msg.set("text", "abc");
        std::cout << msg.get("text").as<capnp::Text>().cStr() << std::endl;

        return kj::READY_NOW;
    }

private:
    GenericInterface::ClientInterface::Client client;
};

class Router_impl final : public GenericInterface::Server
{
public:
    kj::Promise<void> registerInterface(RegisterInterfaceContext context) override
    {
        std::cout << "register interface\n";
        auto client = context.getParams().getClient();
        auto results = context.getResults();
        results.setLoader(kj::heap<Loader_impl>(client));
        return kj::READY_NOW;
    }

    // private:
    //     GenericInterface::ClientInterface::Client client;
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

    auto &&server = capnp::TwoPartyServer{kj::heap<Router_impl>()};
    auto serv = server.listen(*listener);
    serv.wait(wait_scope);
}
