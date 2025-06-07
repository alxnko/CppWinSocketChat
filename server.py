import asyncio

clients = {}


async def broadcast_message(message, sender_writer):
    # Iterate over a copy of the client keys to avoid modification issues
    for writer in list(clients.keys()):
        if writer != sender_writer:
            try:
                writer.write(message.encode())
                await writer.drain()
            except Exception:
                writer.close()
                await writer.wait_closed()
                # Remove the failed client
                del clients[writer]


async def handle_client(reader, writer):
    # Send welcome message
    writer.write("Welcome!\n".encode())
    await writer.drain()

    # Get client's username
    username = (await reader.readline()).decode().strip()
    clients[writer] = username
    join_msg = f"[{username}] joined the chat.\n"
    print(join_msg.strip())
    await broadcast_message(join_msg, writer)

    try:
        while True:
            data = await reader.readline()
            if not data:
                break
            message = data.decode().strip()
            if not message:
                continue
            full_message = f"[{username}] {message}\n"
            print(full_message.strip())
            await broadcast_message(full_message, writer)
    finally:
        leave_msg = f"[{username}] left the chat.\n"
        print(leave_msg.strip())
        # Remove the client before closing
        del clients[writer]
        writer.close()
        await writer.wait_closed()
        await broadcast_message(leave_msg, writer)


async def main():
    server = await asyncio.start_server(handle_client, host="0.0.0.0", port=44444)
    print("Server started on port 44444. Waiting for clients...")
    async with server:
        await server.serve_forever()


if __name__ == "__main__":
    asyncio.run(main())
