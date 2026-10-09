// Differential check for the native Field Mouse Rough.js line port.
// 1. bash reference-code/fieldmouse/run.sh --parity /tmp/rough-ops.json
// 2. node reference-code/roughjs/check-parity.mjs /tmp/rough-ops.json
import { readFileSync } from 'node:fs';
import { roughLineReference } from './line-reference.mjs';

const filename = process.argv[2];
if (!filename) {
  console.error('usage: node check-parity.mjs FIELD_MOUSE_OPS_JSON');
  process.exit(64);
}
const actual = JSON.parse(readFileSync(filename, 'utf8'));
const expected = roughLineReference(10, 20, 100, 180, 17);
if (!Array.isArray(actual) || actual.length !== expected.length) {
  throw new Error('wrong operation count');
}
let maxError = 0;
for (let i = 0; i < expected.length; i++) {
  if (actual[i].op !== expected[i].op ||
      actual[i].data.length !== expected[i].data.length) {
    throw new Error(`different operation type at ${i}`);
  }
  for (let j = 0; j < expected[i].data.length; j++) {
    const diff = Math.abs(actual[i].data[j] - expected[i].data[j]);
    if (!Number.isFinite(diff)) throw new Error('nonfinite coordinate');
    maxError = Math.max(maxError, diff);
  }
}
if (maxError > 1e-7) throw new Error(`coordinate mismatch: ${maxError}`);
console.log(`PASS Rough.js seeded line algorithm parity (max error ${maxError})`);
