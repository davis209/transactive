using System;
using System.Diagnostics;
using System.Drawing;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TrainVoiceRestClientSimulator
{
    public sealed class MainForm : Form
    {
        private readonly TextBox baseUrl = new TextBox { Text = "http://127.0.0.1:8094", Width = 240 };
        private readonly ComboBox templates = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList, Width = 220 };
        private readonly NumericUpDown train = new NumericUpDown { Minimum = 1, Maximum = 254, Value = 1, Width = 55 };
        private readonly TextBox callId = new TextBox { Width = 260, Text = Guid.NewGuid().ToString() };
        private readonly TextBox requestId = new TextBox { Width = 110, Text = "1" };
        private readonly ComboBox method = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList, Width = 90 };
        private readonly TextBox path = new TextBox { Dock = DockStyle.Fill };
        private readonly TextBox body = new TextBox { Multiline = true, ScrollBars = ScrollBars.Both, AcceptsReturn = true, AcceptsTab = true, WordWrap = false, Dock = DockStyle.Fill, Font = new Font("Consolas", 10) };
        private readonly TextBox output = new TextBox { Multiline = true, ScrollBars = ScrollBars.Both, ReadOnly = true, WordWrap = false, Dock = DockStyle.Fill, Font = new Font("Consolas", 10) };
        private readonly Button send = new Button { Text = "发送请求", AutoSize = true, Height = 30 };
        private readonly HttpClient client = new HttpClient { Timeout = TimeSpan.FromSeconds(15) };
        private static Label LabelOf(string s) { return new Label { Text = s, AutoSize = true, Padding = new Padding(2, 7, 0, 0) }; }
        private static FlowLayoutPanel Flow() { return new FlowLayoutPanel { Dock = DockStyle.Fill, WrapContents = true, AutoScroll = true, Padding = new Padding(4) }; }
        private static GroupBox Group(string s, Control child) { var g = new GroupBox { Text = s, Dock = DockStyle.Fill, Padding = new Padding(7) }; g.Controls.Add(child); return g; }
        private static Button ButtonOf(string s, EventHandler h) { var b = new Button { Text = s, AutoSize = true, Height = 30 }; b.Click += h; return b; }
        public MainForm()
        {
            Text = "TrainVoiceAgent REST 接口客户端模拟器"; ClientSize = new Size(1200, 800); MinimumSize = new Size(900, 670); StartPosition = FormStartPosition.CenterScreen;
            Font = new Font("Microsoft YaHei UI", 9F);
            var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 5, Padding = new Padding(8) };
            layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 125)); layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 70)); layout.RowStyles.Add(new RowStyle(SizeType.Percent, 43)); layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 46)); layout.RowStyles.Add(new RowStyle(SizeType.Percent, 57)); Controls.Add(layout);
            var config = Flow();
            templates.Items.AddRange(new object[] { "健康检查", "查询列车", "查询当前通话", "创建通话", "删除通话", "查询待处理通话请求", "删除通话请求", "0x01 音频请求", "0x25 自由文本", "0x0A 列车控制", "0x0B 时间与编组", "0x04 PI 命令" });
            config.Controls.AddRange(new Control[] { LabelOf("服务地址"), baseUrl, LabelOf("接口模板"), templates, ButtonOf("载入模板", (s,e) => ApplyTemplate()), LabelOf("列车 ID"), train, LabelOf("callId"), callId, ButtonOf("新建 UUID", (s,e) => callId.Text = Guid.NewGuid().ToString()), LabelOf("requestId"), requestId });
            layout.Controls.Add(Group("1  服务和模板参数", config), 0, 0);
            var address = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 3 }; address.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90)); address.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100)); address.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 100));
            method.Items.AddRange(new object[] { "GET", "POST", "DELETE", "PUT" }); method.SelectedIndex = 0; address.Controls.Add(method,0,0); address.Controls.Add(path,1,0); address.Controls.Add(send,2,0); send.Click += async (s,e) => await SendRequest();
            layout.Controls.Add(Group("2  HTTP 方法和路径（可编辑）", address), 0, 1);
            layout.Controls.Add(Group("3  请求 JSON（可编辑）", body), 0, 2);
            var tools = Flow(); tools.Controls.Add(ButtonOf("清空结果", (s,e) => output.Clear())); tools.Controls.Add(LabelOf("HTTP 202 仅表示已发送 UDP；音频请求可能等待列车回复。")); layout.Controls.Add(tools,0,3);
            layout.Controls.Add(Group("4  请求与响应记录", output), 0, 4);
            templates.SelectedIndex = 0; ApplyTemplate(); FormClosed += (s,e) => client.Dispose();
        }
        private void ApplyTemplate()
        {
            string root = "/api/train-voice/", target = root + "trains/" + train.Value + "/";
            string id = callId.Text.Trim(); string json = ""; string endpoint = ""; string verb = "GET";
            switch (templates.SelectedIndex)
            {
                case 0: endpoint = "/health"; break;
                case 1: endpoint = root + "trains"; break;
                case 2: endpoint = root + "active-calls"; break;
                case 3: verb = "POST"; endpoint = root + "active-calls"; json = "{\r\n  \"callId\": \"" + id + "\",\r\n  \"trainId\": " + train.Value + ",\r\n  \"carId\": 101,\r\n  \"deviceKey\": 1,\r\n  \"audioFunction\": 6,\r\n  \"sipExtension\": \"1001\",\r\n  \"sipCallId\": \"example-sip-call-id\",\r\n  \"clientId\": \"web-client-1\"\r\n}"; break;
                case 4: verb = "DELETE"; endpoint = root + "active-calls/" + Uri.EscapeDataString(id); break;
                case 5: endpoint = root + "call-requests"; break;
                case 6: verb = "DELETE"; endpoint = root + "call-requests/" + Uri.EscapeDataString(requestId.Text.Trim()); break;
                case 7: verb = "POST"; endpoint = target + "audio-requests"; json = "{\r\n  \"carId\": 101,\r\n  \"function\": 2,\r\n  \"start\": true,\r\n  \"additional\": 0\r\n}"; break;
                case 8: verb = "POST"; endpoint = target + "live-text"; json = "{\r\n  \"carId\": 101,\r\n  \"text\": \"列车测试\"\r\n}"; break;
                case 9: verb = "POST"; endpoint = target + "md-commands"; json = "{\r\n  \"carId\": 101,\r\n  \"do0\": 0,\r\n  \"do1\": 0\r\n}"; break;
                case 10: verb = "POST"; endpoint = target + "train-information"; json = "{\r\n  \"carId\": 101,\r\n  \"timestamp\": \"" + DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ss") + "\",\r\n  \"carIds\": [101, 102]\r\n}"; break;
                case 11: verb = "POST"; endpoint = target + "pi-commands"; json = "{\r\n  \"carId\": 101,\r\n  \"command\": 1\r\n}"; break;
            }
            method.SelectedItem = verb; path.Text = endpoint; body.Text = json;
        }
        private async Task SendRequest()
        {
            Uri root, uri;
            if (!Uri.TryCreate(baseUrl.Text.Trim().TrimEnd('/') + "/", UriKind.Absolute, out root) || (root.Scheme != Uri.UriSchemeHttp && root.Scheme != Uri.UriSchemeHttps) ||
                !Uri.TryCreate(root, path.Text.Trim().TrimStart('/'), out uri) || uri.Host != root.Host || uri.Port != root.Port || uri.Scheme != root.Scheme)
            { MessageBox.Show(this, "请输入有效的 HTTP(S) 服务地址和相对路径。", "地址错误"); return; }
            string verb = Convert.ToString(method.SelectedItem), content = body.Text;
            var watch = Stopwatch.StartNew(); send.Enabled = false;
            try
            {
                Append("[" + DateTime.Now.ToString("HH:mm:ss.fff") + "] >>> " + verb + " " + uri + "\r\n" + (content.Length > 0 ? content + "\r\n" : ""));
                using (var request = new HttpRequestMessage(new HttpMethod(verb), uri))
                {
                    if (verb == "POST" || verb == "PUT" || content.Length > 0) request.Content = new StringContent(content, Encoding.UTF8, "application/json");
                    using (var response = await client.SendAsync(request))
                    {
                        string responseBody = await response.Content.ReadAsStringAsync();
                        Append("<<< HTTP " + (int)response.StatusCode + " " + response.ReasonPhrase + "  " + watch.ElapsedMilliseconds + " ms\r\n" + response.Headers + response.Content.Headers + (responseBody.Length > 0 ? "\r\n" + responseBody : "\r\n(无响应内容)") + "\r\n\r\n");
                    }
                }
            }
            catch (Exception ex) { Append("<<< 请求失败（" + watch.ElapsedMilliseconds + " ms）: " + ex.Message + "\r\n\r\n"); }
            finally { if (!IsDisposed) send.Enabled = true; }
        }
        private void Append(string text) { if (!IsDisposed) { output.AppendText(text); } }
    }
}

