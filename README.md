# 🎧 Application Audio Capture (Windows Only)

N-API port of the WASAPI process-loopback sample to get a nodejs Buffer of an application's audio - or of the entire system's audio.

> ⚠️ Requirements:
> This package only runs on Windows 10 x64 and later. It uses native binaries and will warn on unsupported platforms.

### 🚀 Features

- Capture raw PCM audio from individual applications using WASAPI process-loopback.
- Capture raw PCM audio from the entire system (whatever's playing through the default output device) - no process ID required.
- Pipe real-time audio data into your JavaScript/TypeScript app.

https://github.com/user-attachments/assets/fc058596-6ea3-4ded-8065-aeb642c5a465

### 📦 Installation

Install using your favorite package manager.

```sh
npm install loopback-capture
#OR
bun install loopback-capture
#OR any package manager..
```

### 🧠 Usage

1. Start Capturing Audio from a Single Process

```ts
import loopback from "loopback-capture";

const capture = new loopback.LoopbackCapture();
const processId = capture.start(1234, /* includeProcessTree */ true, (chunk: Buffer) => {
  console.log("Application audio data:", chunk); // Buffer
});

process.on("SIGINT", () => {
  capture.stop();
  process.exit(0);
});
```

> 🧠 chunk is a raw PCM audio buffer. You can pipe it to a file, stream it, analyze it, etc.

2. Start Capturing System-Wide Audio (no PID needed)

Want everything the system is playing - not just one app? Use `startSystemAudio` instead. It skips process filtering entirely and captures straight from the default playback device.

```ts
import loopback from "loopback-capture";

const capture = new loopback.LoopbackCapture();
capture.startSystemAudio((chunk: Buffer) => {
  console.log("System audio data:", chunk); // Buffer
});

process.on("SIGINT", () => {
  capture.stop();
  process.exit(0);
});
```

> 🧠 Same raw PCM format as process capture (16-bit, stereo, 48kHz) - just sourced from the whole desktop instead of one process tree.

### 🪟 Why and how?

For my desktop application ([Huginn](https://github.com/WerdoxDev/Huginn)) I needed to capture an application's audio selectively to share in a call just like discord does. I later added full system audio capture too, for cases where selecting one process isn't the point - like recording the whole desktop for a meeting.

The two modes use different Windows APIs under the hood:

- **Per-process capture** uses `AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK` (Windows 10 2004+), activated asynchronously via `ActivateAudioInterfaceAsync`, and can optionally include a process's child tree.
- **System-wide capture** uses the classic WASAPI loopback-recording technique - grabbing the current default playback device via `IMMDeviceEnumerator` and activating an `IAudioClient` on it directly with `AUDCLNT_STREAMFLAGS_LOOPBACK` - the same approach tools like OBS use for desktop audio.

Most of the C++ source is simply a stripped out version from a sample in microsoft's classic samples repo
https://github.com/microsoft/Windows-classic-samples

### 🧪 Example Use Cases

- Build a real-time audio visualizer for specific apps.
- Record browser or game audio selectively.
- Stream audio from only one process instead of the whole system.
- Record full desktop/system audio for meeting or screen recordings.
