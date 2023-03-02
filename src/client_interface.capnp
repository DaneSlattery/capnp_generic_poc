@0xa5fe33e471b84055;

struct Message{
    text @0 :Text;
    timestamp @1 :Timestamp;
}

struct Timestamp{
    nanoseconds @0 :UInt64;
}
