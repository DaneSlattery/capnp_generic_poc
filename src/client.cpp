
#include <capnp/rpc-twoparty.h>
#include <capnp/ez-rpc.h>
#include <capnp/schema.h>
#include <capnp/schema-loader.h>

#include <iostream>
#include <kj/async-io.h>

#include "generic.capnp.h"
#include "client_interface.capnp.h"

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
    auto cap = client.bootstrap().castAs<InterfaceLoader>();

    capnp::MallocMessageBuilder mb;
    auto message = mb.getRoot<UserSchema>();
    message.setTypeId(capnp::typeId<Message>());
    capnp::SchemaLoader loader;

    loader.loadCompiledTypeAndDependencies<Message>();

    auto schemas = loader.getAllLoaded();
    auto listBuilder = message.initSchemas(schemas.size());
    for (auto i : kj::indices(schemas))
    {
        listBuilder.setWithCaveats(i, schemas[i].getProto());
    }

    auto request = cap.loadRequest();
    request.setSchema(message.asReader());
    auto promise = request.send();

    // // Wait for the result.  This is the only line that blocks.
    auto response = promise.wait(waitScope);

    auto msg = response.getValue().as<Message>();
    std::cout << msg.getText().cStr() << std::endl;
    std::cout << msg.getTimestamp().getNanoseconds() << std::endl;

    while (true)
    {
        //
    }
}
