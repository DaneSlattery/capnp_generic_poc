@0x9fb7648c51432e45;

using Schema = import "/capnp/schema.capnp";


struct Struct {
	userSchema @0 :Schema.Node;
}


interface GenericInterface{
    interface InterfaceLoader
    {   
        load @0 (struct :Struct) -> ();
    }
    
    interface ClientInterface
    {
        call @0 (value :AnyStruct) -> (echo :AnyStruct);
    }

    registerInterface @0 (client :ClientInterface) -> (loader :InterfaceLoader);
}
