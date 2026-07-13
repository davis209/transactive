namespace PABridgeAgentTestClient
{
    public sealed class EndpointDefinition
    {
        public EndpointDefinition(string name, string method, string pathTemplate, string sampleBody)
        {
            Name = name;
            Method = method;
            PathTemplate = pathTemplate;
            SampleBody = sampleBody;
        }

        public string Name { get; private set; }
        public string Method { get; private set; }
        public string PathTemplate { get; private set; }
        public string SampleBody { get; private set; }

        public override string ToString()
        {
            return Method + " " + Name;
        }
    }
}
