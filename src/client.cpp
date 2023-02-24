
#include <capnp/rpc-twoparty.h>
#include <capnp/ez-rpc.h>
#include <capnp/schema.h>
#include <iostream>
#include <kj/async-io.h>

#include "generic.capnp.h"
#include "client_interface.capnp.h"

class ClientImpl final : public GenericInterface::ClientInterface::Server
{
public:
    kj::Promise<void> call(CallContext context) override
    {
        std::cout << "call\n";
        return kj::READY_NOW;
    }
};

int main(int argc, const char *argv[])
{
    // We expect one argument specifying the server address.
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " HOST[:PORT]" << std::endl;
        return 1;
    }

    auto bind_address = argv[1];

    auto asyncio = kj::setupAsyncIo();
    auto &waitScope = asyncio.waitScope;

    auto networkAddress = asyncio.provider->getNetwork().parseAddress(bind_address).wait(waitScope);
    auto connection = networkAddress->connect().wait(waitScope);

    auto &&client = capnp::TwoPartyClient{*connection};
    auto cap = client.bootstrap().castAs<GenericInterface>();

    auto request = cap.registerInterfaceRequest();
    request.setClient(kj::heap<ClientImpl>());
    auto result = request.send().wait(waitScope);
    auto loader = result.getLoader();

    auto load_request = loader.loadRequest();
    auto struct_ = load_request.initStruct();
    struct_.setUserSchema(capnp::Schema::from<Message>().getProto());
    auto promise = load_request.send();

    // // Wait for the result.  This is the only line that blocks.
    auto response = promise.wait(waitScope);

    while (true)
    {
        //
    }
}
