import asyncio
import socket
import time
import capnp
import pathlib
import sys
import capnp

_schema_path = str(
    (pathlib.Path(__file__).parent / pathlib.Path("src/")).resolve().absolute()
)
sys.path.append(_schema_path)

import generic_capnp


class Server:
    async def myreader(self):
        while self.retry:
            try:
                # Must be a wait_for so we don't block on read()
                data = await asyncio.wait_for(self.reader.read(4096), timeout=0.1)
            except asyncio.TimeoutError:
                print("myreader timeout.")
                continue
            except Exception as err:
                print("Unknown myreader err: %s", err)
                return False
            await self.server.write(data)
        print("myreader done.")
        return True

    async def mywriter(self):
        while self.retry:
            try:
                # Must be a wait_for so we don't block on read()
                data = await asyncio.wait_for(self.server.read(4096), timeout=0.1)
                self.writer.write(data.tobytes())
            except asyncio.TimeoutError:
                print("mywriter timeout.")
                continue
            except Exception as err:
                print("Unknown mywriter err: %s", err)
                return False
        print("mywriter done.")
        return True

    async def myserver(self, reader, writer):
        # Start TwoPartyServer using TwoWayPipe (only requires bootstrap)
        self.server = capnp.TwoPartyServer(bootstrap=LoaderImpl())
        self.reader = reader
        self.writer = writer
        self.retry = True

        # Assemble reader and writer tasks, run in the background
        coroutines = [self.myreader(), self.mywriter()]
        tasks = asyncio.gather(*coroutines, return_exceptions=True)

        while True:
            self.server.poll_once()
            # Check to see if reader has been sent an eof (disconnect)
            if self.reader.at_eof():
                self.retry = False
                break
            await asyncio.sleep(0.01)

        # Make wait for reader/writer to finish (prevent possible resource leaks)
        await tasks


class LoaderImpl(generic_capnp.InterfaceLoader.Server):
    def load_context(self, _context, **kwargs):
        client_schema = _context.params.clientSchema

        schema_loader = capnp.SchemaLoader()
        for schema in client_schema.schemas:
            schema_loader.load_dynamic(schema)

        root_schema = schema_loader.get(client_schema.typeId)

        results = _context.results

        value = results.value.as_struct(root_schema.as_struct())

        value.text = "TEXT!"
        value.timestamp.nanoseconds = 15


async def new_connection(reader, writer):
    server = Server()
    await server.myserver(reader, writer)


async def main():
    addr = "localhost"
    port = "1234"

    # Handle both IPv4 and IPv6 cases
    try:
        print("Try IPv4")
        server = await asyncio.start_server(
            new_connection, addr, port, family=socket.AF_INET
        )
    except Exception:
        print("Try IPv6")
        server = await asyncio.start_server(
            new_connection, addr, port, family=socket.AF_INET6
        )

    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
