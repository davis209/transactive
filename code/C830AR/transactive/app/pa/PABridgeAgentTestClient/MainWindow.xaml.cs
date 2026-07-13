using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace PABridgeAgentTestClient
{
    public partial class MainWindow : Window
    {
        private readonly List<EndpointDefinition> _endpoints;

        public MainWindow()
        {
            InitializeComponent();
            _endpoints = CreateEndpoints();
            EndpointListBox.ItemsSource = _endpoints;
            EndpointListBox.SelectedIndex = 0;
        }

        private static List<EndpointDefinition> CreateEndpoints()
        {
            return new List<EndpointDefinition>
            {
                new EndpointDefinition("Health", "GET", "/health", ""),
                new EndpointDefinition("Broadcast list", "GET", "/api/pa/broadcasts", ""),
                new EndpointDefinition("Broadcast config", "GET", "/api/pa/broadcasts/{broadcastId}/config", ""),
                new EndpointDefinition("Broadcast progress", "GET", "/api/pa/broadcasts/{broadcastId}/progress", ""),
                new EndpointDefinition("Terminate broadcast", "POST", "/api/pa/broadcasts/{broadcastId}/terminate", "{}"),
                new EndpointDefinition("Remove broadcast", "POST", "/api/pa/broadcasts/{broadcastId}/remove", "{}"),
                new EndpointDefinition("Change broadcast id", "POST", "/api/pa/broadcasts/{broadcastId}/change-id", "{\r\n  \"toBroadcastId\": \"NewBroadcastId\"\r\n}"),
                new EndpointDefinition("Retry station broadcast", "POST", "/api/pa/broadcasts/{broadcastId}/retry-station", "{\r\n  \"zones\": [1, 2, 3]\r\n}"),
                new EndpointDefinition("Retry train broadcast", "POST", "/api/pa/broadcasts/{broadcastId}/retry-train", "{\r\n  \"trains\": [1, 2],\r\n  \"hasOverrideOption\": false\r\n}"),
                new EndpointDefinition("Station music", "POST", "/api/pa/station/music", "{\r\n  \"musicType\": \"Occ\"\r\n}"),
                new EndpointDefinition("Station music status", "GET", "/api/pa/station/music-status", ""),
                new EndpointDefinition("Station DVA", "POST", "/api/pa/station/dva", "{\r\n  \"messages\": [1001],\r\n  \"zones\": [1, 2],\r\n  \"dwellInSecs\": 0,\r\n  \"hasChime\": true,\r\n  \"startTime\": 0,\r\n  \"stopTime\": 0,\r\n  \"periodInSecs\": 0,\r\n  \"isSynchronisedWithTis\": false,\r\n  \"groupName\": \"\"\r\n}"),
                new EndpointDefinition("Station live", "POST", "/api/pa/station/live", "{\r\n  \"sourceId\": 1,\r\n  \"zones\": [1, 2],\r\n  \"groupName\": \"\"\r\n}"),
                new EndpointDefinition("Record adhoc", "POST", "/api/pa/station/record-adhoc", "{\r\n  \"sourceId\": 1,\r\n  \"messageKey\": 1001\r\n}"),
                new EndpointDefinition("Set adhoc type", "POST", "/api/pa/station/adhoc-type", "{\r\n  \"messageKey\": 1001,\r\n  \"messageType\": \"N\"\r\n}"),
                new EndpointDefinition("Set adhoc label", "POST", "/api/pa/station/adhoc-label", "{\r\n  \"messageKey\": 1001,\r\n  \"label\": \"Test message\"\r\n}"),
                new EndpointDefinition("Train DVA", "POST", "/api/pa/train/dva", "{\r\n  \"trains\": [1, 2],\r\n  \"messageKey\": 1001,\r\n  \"libraryVersion\": 1,\r\n  \"announcementPeriod\": 0,\r\n  \"announcementDuration\": 0,\r\n  \"hasOverrideOption\": false\r\n}"),
                new EndpointDefinition("Train live", "POST", "/api/pa/train/live", "{\r\n  \"trains\": [1, 2],\r\n  \"hasOverrideOption\": false,\r\n  \"staticGroup\": \"\"\r\n}"),
                new EndpointDefinition("Train live begin", "POST", "/api/pa/train-live/{broadcastId}/begin", "{}"),
                new EndpointDefinition("Train live continue", "POST", "/api/pa/train-live/{broadcastId}/continue", "{}"),
                new EndpointDefinition("Train live end", "POST", "/api/pa/train-live/{broadcastId}/end", "{}"),
                new EndpointDefinition("Priority scheme", "GET", "/api/pa/priority-scheme", ""),
                new EndpointDefinition("Station DVA messages", "GET", "/api/pa/config/station-dva-messages", ""),
                new EndpointDefinition("Train DVA messages", "GET", "/api/pa/config/train-dva-messages", ""),
                new EndpointDefinition("DVA public versions", "GET", "/api/pa/config/dva-public-versions", ""),
                new EndpointDefinition("DVA private versions", "GET", "/api/pa/config/dva-private-versions", ""),
                new EndpointDefinition("PA zones", "GET", "/api/pa/config/zones", "")
            };
        }

        private void EndpointListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            LoadSelectedSample();
            UpdateResolvedUrl();
        }

        private void LoadSampleButton_Click(object sender, RoutedEventArgs e)
        {
            LoadSelectedSample();
        }

        private async void HealthButton_Click(object sender, RoutedEventArgs e)
        {
            await SendAsync(new EndpointDefinition("Health", "GET", "/health", ""));
        }

        private async void SendButton_Click(object sender, RoutedEventArgs e)
        {
            var endpoint = EndpointListBox.SelectedItem as EndpointDefinition;
            if (endpoint == null)
            {
                MessageBox.Show(this, "Please select an API first.", "PABridgeAgent Test Client", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            await SendAsync(endpoint);
        }

        private void LoadSelectedSample()
        {
            var endpoint = EndpointListBox.SelectedItem as EndpointDefinition;
            if (endpoint == null)
            {
                return;
            }

            RequestBodyTextBox.Text = endpoint.SampleBody;
        }

        private void UpdateResolvedUrl()
        {
            var endpoint = EndpointListBox.SelectedItem as EndpointDefinition;
            ResolvedUrlTextBox.Text = endpoint == null ? "" : BuildUrl(endpoint);
        }

        private async Task SendAsync(EndpointDefinition endpoint)
        {
            StatusTextBlock.Text = "Sending...";
            ResponseTextBox.Text = "";
            UpdateResolvedUrl();

            try
            {
                string response = await ExecuteRequestAsync(endpoint);
                ResponseTextBox.Text = response;
            }
            catch (WebException ex)
            {
                ResponseTextBox.Text = ReadWebException(ex);
            }
            catch (Exception ex)
            {
                ResponseTextBox.Text = ex.ToString();
                StatusTextBlock.Text = "Failed";
            }
        }

        private async Task<string> ExecuteRequestAsync(EndpointDefinition endpoint)
        {
            string url = BuildUrl(endpoint);
            int timeout = ParseTimeout();
            var request = (HttpWebRequest)WebRequest.Create(url);
            request.Method = endpoint.Method;
            request.Accept = "application/json";
            request.Timeout = timeout;
            request.ReadWriteTimeout = timeout;

            string body = RequestBodyTextBox.Text ?? "";
            if (endpoint.Method == "POST")
            {
                byte[] payload = Encoding.UTF8.GetBytes(body);
                request.ContentType = "application/json; charset=utf-8";
                request.ContentLength = payload.Length;
                using (Stream requestStream = await request.GetRequestStreamAsync())
                {
                    await requestStream.WriteAsync(payload, 0, payload.Length);
                }
            }

            using (var response = (HttpWebResponse)await request.GetResponseAsync())
            using (var stream = response.GetResponseStream())
            using (var reader = new StreamReader(stream ?? Stream.Null, Encoding.UTF8))
            {
                StatusTextBlock.Text = ((int)response.StatusCode) + " " + response.StatusDescription;
                return await reader.ReadToEndAsync();
            }
        }

        private string BuildUrl(EndpointDefinition endpoint)
        {
            string baseUrl = (BaseUrlTextBox.Text ?? "").Trim();
            if (baseUrl.EndsWith("/", StringComparison.Ordinal))
            {
                baseUrl = baseUrl.Substring(0, baseUrl.Length - 1);
            }

            string path = endpoint.PathTemplate.Replace("{broadcastId}", Uri.EscapeDataString(BroadcastIdTextBox.Text ?? ""));
            return baseUrl + path;
        }

        private int ParseTimeout()
        {
            int timeout;
            if (!int.TryParse(TimeoutTextBox.Text, out timeout) || timeout <= 0)
            {
                return 30000;
            }
            return timeout;
        }

        private string ReadWebException(WebException ex)
        {
            var response = ex.Response as HttpWebResponse;
            if (response == null)
            {
                StatusTextBlock.Text = "Failed";
                return ex.ToString();
            }

            StatusTextBlock.Text = ((int)response.StatusCode) + " " + response.StatusDescription;
            using (response)
            using (var stream = response.GetResponseStream())
            using (var reader = new StreamReader(stream ?? Stream.Null, Encoding.UTF8))
            {
                return reader.ReadToEnd();
            }
        }
    }
}
