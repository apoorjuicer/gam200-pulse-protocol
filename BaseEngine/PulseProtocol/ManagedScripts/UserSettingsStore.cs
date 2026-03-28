using System;
using System.IO;
using System.Text.Json;

internal static class UserSettingsStore
{
    private const string SaveFileName = "user_settings.json";

    private static readonly string[] CandidatePaths = new[]
    {
        Path.Combine("PulseProtocol", "JSON", SaveFileName),
        Path.Combine("..", "..", "PulseProtocol", "JSON", SaveFileName),
        Path.Combine("..", "..", "..", "PulseProtocol", "JSON", SaveFileName),
        SaveFileName
    };

    private sealed class Data
    {
        public float Volume { get; set; } = 1.0f;
        public float Music { get; set; } = 1.0f;
        public float SFX { get; set; } = 0.25f;
        public float Brightness { get; set; } = 1.0f;
        public float Latency { get; set; } = 0.5f;
        public bool Fullscreen { get; set; } = false;
    }

    private static bool loaded = false;
    private static Data cached = new Data();

    public static float GetSliderValue(string tag, float fallback)
    {
        EnsureLoaded();
        string key = NormalizeTag(tag);
        if (key == "volume") return Clamp01(cached.Volume);
        if (key == "music") return Clamp01(cached.Music);
        if (key == "sfx") return Clamp01(cached.SFX);
        if (key == "brightness") return Clamp01(cached.Brightness);
        if (key == "latency") return Clamp01(cached.Latency);
        return Clamp01(fallback);
    }

    public static void SetSliderValue(string tag, float value01)
    {
        EnsureLoaded();

        float v = Clamp01(value01);
        bool changed = false;
        string key = NormalizeTag(tag);

        if (key == "volume" && Math.Abs(cached.Volume - v) > 0.0005f) { cached.Volume = v; changed = true; }
        else if (key == "music" && Math.Abs(cached.Music - v) > 0.0005f) { cached.Music = v; changed = true; }
        else if (key == "sfx" && Math.Abs(cached.SFX - v) > 0.0005f) { cached.SFX = v; changed = true; }
        else if (key == "brightness" && Math.Abs(cached.Brightness - v) > 0.0005f) { cached.Brightness = v; changed = true; }
        else if (key == "latency" && Math.Abs(cached.Latency - v) > 0.0005f) { cached.Latency = v; changed = true; }

        if (changed)
            Save();
    }

    public static bool GetFullscreen()
    {
        EnsureLoaded();
        return cached.Fullscreen;
    }

    public static void SetFullscreen(bool fullscreen)
    {
        EnsureLoaded();
        if (cached.Fullscreen == fullscreen)
            return;

        cached.Fullscreen = fullscreen;
        Save();
    }

    private static string NormalizeTag(string tag)
    {
        if (string.IsNullOrEmpty(tag))
            return "";
        return tag.Trim().ToLowerInvariant();
    }

    private static float Clamp01(float v)
    {
        if (v < 0f) return 0f;
        if (v > 1f) return 1f;
        return v;
    }

    private static void EnsureLoaded()
    {
        if (loaded)
            return;

        loaded = true;

        foreach (string path in CandidatePaths)
        {
            try
            {
                if (!File.Exists(path))
                    continue;

                string json = File.ReadAllText(path);
                Data? data = JsonSerializer.Deserialize<Data>(json);
                if (data != null)
                {
                    cached = data;
                    return;
                }
            }
            catch
            {
                // Keep defaults if bad file/path.
            }
        }
    }

    private static void Save()
    {
        string json = JsonSerializer.Serialize(cached, new JsonSerializerOptions { WriteIndented = true });

        foreach (string path in CandidatePaths)
        {
            try
            {
                string? dir = Path.GetDirectoryName(path);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    continue;

                File.WriteAllText(path, json);
                return;
            }
            catch
            {
                // Try next candidate.
            }
        }
    }
}
