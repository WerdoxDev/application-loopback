# 🎧 Application Audio Capture (Windows Only)

A simple Node.js wrapper around native C++ binaries to list running application windows and capture audio from specific Windows processes using loopback recording.

>   ⚠️ Requirements:
   This package only runs on Windows 10 x64 and later. It uses native binaries and will throw an error on unsupported platforms.

### 🚀 Features

- List all visible application windows and their process IDs.

- Capture raw PCM audio from individual applications using WASAPI loopback.

- Pipe real-time audio data into your JavaScript/TypeScript app.


📦 Installation

Install using your favourite package manager.
```
npm install application-loopback
//OR
bun install application-loopback
```

🧠 Usage
1. Get Active Window Titles and Process IDs

import { getActiveWindowProcessIds } from "your-package-name";

const windows = await getActiveWindowProcessIds();

windows.forEach(win => {
  console.log(`PID: ${win.processId}, Title: ${win.title}`);
});

2. Start Capturing Audio from a Process

import { startAudioCapture } from "your-package-name";

startAudioCapture("1234", {
  onData: (chunk) => {
    console.log("Audio data:", chunk); // Uint8Array
  },
});

    🧠 chunk is a raw PCM audio buffer. You can pipe it to a file, stream it, analyze it, etc.

3. Stop Capturing Audio

import { stopAudioCapture } from "your-package-name";

const stopped = stopAudioCapture("1234");

if (stopped) {
  console.log("Audio capture stopped.");
} else {
  console.log("No capture process found for that PID.");
}

🪟 Why Native Binaries?

Capturing audio from other apps requires low-level access that Node.js alone can’t handle. This project uses small C++ utilities under the hood to:

    Query visible window processes.

    Access per-process loopback audio streams.

🧪 Example Use Cases

    Build a real-time audio visualizer for specific apps.

    Record browser or game audio selectively.

    Stream audio from only one process instead of the whole system.
