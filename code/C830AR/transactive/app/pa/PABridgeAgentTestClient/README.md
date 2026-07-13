# PABridgeAgentTestClient

WPF test client for manually exercising the PABridgeAgent REST API.

## Build

Open `PABridgeAgentTestClient.sln` with Visual Studio 2017 and build the solution.

The project targets .NET Framework 4.6.1 and does not require NuGet packages.

## Use

1. Start PABridgeAgent.
2. Set `Base URL`, for example `http://127.0.0.1:8088`.
3. Select an endpoint.
4. Edit `Broadcast ID` or JSON body as needed.
5. Click `Send`.

PABridgeAgent supplies the Authentication session internally. This client does
not send `sessionId` in REST requests.
