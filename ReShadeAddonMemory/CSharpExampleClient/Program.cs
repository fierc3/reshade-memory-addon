// See https://aka.ms/new-console-template for more information
using System.IO.MemoryMappedFiles;

Console.WriteLine("Starting CSharpExampleClient, this example will");

using var mmf = MemoryMappedFile.CreateOrOpen("Local\\ReShadeAddonShared", sizeof(float));
using var accessor = mmf.CreateViewAccessor();

float alpha = 0.0f;
while (true)
{
    var now = DateTime.UtcNow;
    double seconds = now.TimeOfDay.TotalSeconds;
    alpha = (float)(Math.Sin(seconds * 2.0) * 0.5 + 0.5); // 2 Hz oscillation

    accessor.Write(0, alpha);
    Console.WriteLine("Updated shaared memory");
    Thread.Sleep(60); //60 hz
}