@0x9fb7648c51432e45;

using Schema = import "/capnp/schema.capnp";

struct UserSchema{
    typeId @0 :UInt64;
    schemas @1 :List(Schema.Node);
}

interface InterfaceLoader
{   
    load @0 (schema :UserSchema) -> (value :AnyPointer);
}
