using System.IO.MemoryMappedFiles;
using System.Text;

Console.WriteLine("Starting CSharpExampleClient, this example will");

const int MaxNameLength = 64;

using var mmf = MemoryMappedFile.CreateOrOpen("Local\\ReShadeAddonShared", sizeof(float));
using var accessor = mmf.CreateViewAccessor();

string uniformName = "mem_Alpha";
byte[] nameBytes = Encoding.UTF8.GetBytes(uniformName);
if (nameBytes.Length >= MaxNameLength)
{
    Console.WriteLine("Uniform name too long");
    return;
}

while (true)
{
    double seconds = DateTime.UtcNow.TimeOfDay.TotalSeconds;
    float value = (float)(Math.Sin(seconds * 2.0) * 0.5 + 0.5);

    // Write uniform name (zero-padded)
    accessor.WriteArray(0, nameBytes, 0, nameBytes.Length);
    accessor.Write(MaxNameLength - 1, (byte)0); // null terminator

    // Write value
    accessor.Write(MaxNameLength, value);

    Console.WriteLine($"Sent {uniformName} = {value:F3}");
    Thread.Sleep(16); // ~60Hz
}