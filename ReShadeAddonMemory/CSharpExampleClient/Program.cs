using System.IO.MemoryMappedFiles;
using System.Text;
using SharpHook;

Console.WriteLine("Starting CSharpExampleClient, tracking mouse position in % of screen...");


[System.Runtime.InteropServices.DllImport("user32.dll")]
static extern int GetSystemMetrics(int nIndex);

const int MaxNameLength = 64;
const int EntrySize = MaxNameLength + sizeof(float);
const string MapName = "Local\\ReShadeAddonShared";

// Create shared memory with space for X and Y
using var mmf = MemoryMappedFile.CreateOrOpen(MapName, EntrySize * 2);
using var accessor = mmf.CreateViewAccessor();

string uniformX = "mem_positionX";
string uniformY = "mem_positionY";

byte[] xBytes = Encoding.UTF8.GetBytes(uniformX);
byte[] yBytes = Encoding.UTF8.GetBytes(uniformY);

if (xBytes.Length >= MaxNameLength || yBytes.Length >= MaxNameLength)
{
    Console.WriteLine("Uniform name too long");
    return;
}

var hook = new SimpleGlobalHook();
hook.MouseMoved += (s, e) =>
{
    Console.WriteLine($"Mouse: {e.Data.X}, {e.Data.Y}");

    int screenWidth = GetSystemMetrics(0);  // SM_CXSCREEN
    int screenHeight = GetSystemMetrics(1); // SM_CYSCREEN

    float mouseX = (float)e.Data.X / screenWidth;
    float mouseY = (float)e.Data.Y / screenHeight;

    Console.WriteLine($"Mouse Percentage: {mouseX}, {mouseY}");

    // Write mem_positionX
    accessor.WriteArray(0, xBytes, 0, xBytes.Length);
    accessor.Write(MaxNameLength - 1, (byte)0); // null terminator
    accessor.Write(MaxNameLength, mouseX);

    // Write mem_positionY
    accessor.WriteArray(EntrySize, yBytes, 0, yBytes.Length);
    accessor.Write(EntrySize + MaxNameLength - 1, (byte)0); // null terminator
    accessor.Write(EntrySize + MaxNameLength, mouseY);
};

await hook.RunAsync();
