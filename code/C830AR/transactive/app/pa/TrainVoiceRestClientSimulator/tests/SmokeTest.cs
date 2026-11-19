using System;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows.Forms;
using TrainVoiceRestClientSimulator;

internal static class SmokeTest
{
    private static T Field<T>(MainForm form, string name) { return (T)typeof(MainForm).GetField(name, BindingFlags.Instance | BindingFlags.NonPublic).GetValue(form); }
    [STAThread]
    private static void Main()
    {
        var form = new MainForm { ShowInTaskbar = false, Opacity = 0 };
        int result = 1;
        form.Shown += async (s,e) => {
            try {
                Field<TextBox>(form, "baseUrl").Text = "http://127.0.0.1:39089";
                Field<ComboBox>(form, "templates").SelectedIndex = 7;
                typeof(MainForm).GetMethod("ApplyTemplate", BindingFlags.Instance | BindingFlags.NonPublic).Invoke(form, null);
                if (Field<TextBox>(form, "path").Text != "/api/train-voice/trains/1/audio-requests" || !Field<TextBox>(form, "body").Text.Contains("\"function\": 2")) throw new Exception("Template mismatch");
                await (Task)typeof(MainForm).GetMethod("SendRequest", BindingFlags.Instance | BindingFlags.NonPublic).Invoke(form, null);
                string output = Field<TextBox>(form, "output").Text;
                if (!output.Contains("HTTP 200") || !output.Contains("mock-ok")) throw new Exception(output);
                Console.WriteLine("REST template and HTTP request passed."); result = 0;
            } catch (Exception ex) { Console.Error.WriteLine(ex); }
            form.Close();
        };
        Application.Run(form); Environment.Exit(result);
    }
}
