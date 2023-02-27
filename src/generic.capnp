@0x9fb7648c51432e45;

using Schema = import "/capnp/schema.capnp";

interface InterfaceLoader
{   
    load @0 (schema :Schema.Node) -> (value :AnyStruct);
}
