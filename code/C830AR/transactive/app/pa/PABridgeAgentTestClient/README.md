# PABridgeAgentTestClient

WPF test client for manually exercising the PABridgeAgent REST API.

## Build

Open `PABridgeAgentTestClient.sln` with Visual Studio 2017 and build the solution.

The project targets .NET Framework 4.6.1 and does not require NuGet packages.

## Use

1. Start PABridgeAgent.
2. Set `Base URL`, for example `http://127.0.0.1:8088`.
3. Enter a valid TransActive session ID in `Session ID`.
4. Select an endpoint.
5. Edit `Broadcast ID` or JSON body as needed.
6. Click `Send`.

The client sends the session in the `X-Transactive-Session-Id` header. The
health endpoint does not require a session.
