import chalk from "chalk"
import { getActiveWindowProcessIds, startAudioCapture, stopAudioCapture } from ".";

const dbs: number[] = [0, 0];

async function visualizePCMWithDBPerChannel(
   uint8Array: Uint8Array,
   channels: number,
   fixedChunkSize: number
) {
   const dataView = new DataView(uint8Array.buffer);
   const samples = [];

   for (let i = 0; i < uint8Array.byteLength; i += 2) {
      const sample = dataView.getInt16(i, true);
      samples.push(sample);
   }

   const numChunks = Math.floor(samples.length / channels / fixedChunkSize);

   const calculateDB = (rms: number) => {
      const db = 20 * Math.log10(Math.abs(rms));
      return db;
   };

   for (let i = 0; i < numChunks; i++) {
      for (let channel = 0; channel < channels; channel++) {
         let sumOfSquares = 0;
         for (let j = 0; j < fixedChunkSize; j++) {
            const sampleIndex =
               i * fixedChunkSize * channels + j * channels + channel;
            const sample = samples[sampleIndex];

            if (sample) {
               sumOfSquares += sample * sample;
            }
         }

         const rms = Math.sqrt(sumOfSquares / fixedChunkSize) / 32768;

         const db = calculateDB(rms);
         dbs[channel] = db;
      }
   }
}

function visualizeDb(channel: number, db: number) {
   const MAX_BARS = 50;
   const MAX_DB = -20;
   const MIN_DB = -60;

   const normalizedDb = Math.max(0, Math.min(1, (db - MIN_DB) / (MAX_DB - MIN_DB)));
   const numBars = Math.round(normalizedDb * MAX_BARS);

   const fullBarChar = '#';
   const mediumBarChar = '=';
   const lowBarChar = '-';
   const emptyChar = ' ';

   let barString = '';

   for (let i = 0; i < MAX_BARS; i++) {
      if (i < numBars) {
         if (i < numBars / 3) {
            barString += lowBarChar;
         } else if (i < (numBars * 2) / 3) {
            barString += mediumBarChar;
         } else {
            barString += fullBarChar;
         }
      } else {
         barString += emptyChar;
      }
   }

   console.log(`${chalk.green(channel)} [${chalk.cyan(barString)}] ${chalk.white(db.toFixed(2))} dB`);
}

const window = (await getActiveWindowProcessIds()).find(x => x.title.includes("Audacity"));

if (!window) {
   throw new Error("Spotify process was not found")
}


const spawnedProcess = startAudioCapture(window.processId, {
   onData(data) {
      visualizePCMWithDBPerChannel(data, 2, 1024);
   },
});

setInterval(() => {
   console.clear();
   if (dbs[0] !== undefined && dbs[1] !== undefined) {
      visualizeDb(0, dbs[0]);
      visualizeDb(1, dbs[1]);
   }
}, 50);

process.on("exit", () => {
   stopAudioCapture(spawnedProcess);
})
