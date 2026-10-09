// Independent JavaScript baseline of the Rough.js line generator.
// Adapted from rough-stuff/rough src/renderer.ts and src/math.ts,
// MIT License, Copyright (c) 2019 Preet Shihn.
// See LICENSE.roughjs and the upstream repository.
// This is reference code, not the Swap app renderer.

export function roughLineReference(x1, y1, x2, y2, seed, options = {}) {
  const o = {
    maxRandomnessOffset: 2, roughness: 1, bowing: 1,
    disableMultiStroke: false, preserveVertices: false,
    ...options,
  };
  let state = seed;
  function random() {
    state = Math.imul(48271, state);
    return (2147483647 & state) / 2147483648;
  }
  function offset(min, max, gain = 1) {
    return o.roughness * gain * (random() * (max - min) + min);
  }
  function randomOffset(n, gain = 1) {
    return offset(-n, n, gain);
  }
  function stroke(overlay) {
    const dx = x2 - x1, dy = y2 - y1;
    const squared = dx * dx + dy * dy;
    const length = Math.sqrt(squared);
    const gain = length < 200 ? 1 :
      length > 500 ? 0.4 : -0.0016668 * length + 1.233334;
    let extent = o.maxRandomnessOffset;
    if (extent * extent * 100 > squared) {
      extent = length / 10;
    }
    const half = extent / 2;
    const diverge = 0.2 + random() * 0.2;
    let midX = o.bowing * o.maxRandomnessOffset * (y2 - y1) / 200;
    let midY = o.bowing * o.maxRandomnessOffset * (x1 - x2) / 200;
    midX = randomOffset(midX, gain);
    midY = randomOffset(midY, gain);
    const extentForStroke = overlay ? half : extent;
    const startX = x1 + (o.preserveVertices ? 0 : randomOffset(extentForStroke, gain));
    const startY = y1 + (o.preserveVertices ? 0 : randomOffset(extentForStroke, gain));
    const c1x = midX + x1 + dx * diverge + randomOffset(extentForStroke, gain);
    const c1y = midY + y1 + dy * diverge + randomOffset(extentForStroke, gain);
    const c2x = midX + x1 + 2 * dx * diverge + randomOffset(extentForStroke, gain);
    const c2y = midY + y1 + 2 * dy * diverge + randomOffset(extentForStroke, gain);
    const endX = x2 + (o.preserveVertices ? 0 : randomOffset(extentForStroke, gain));
    const endY = y2 + (o.preserveVertices ? 0 : randomOffset(extentForStroke, gain));
    return [
      { op: 'move', data: [startX, startY] },
      { op: 'bcurveTo', data: [c1x, c1y, c2x, c2y, endX, endY] },
    ];
  }

  const ops = stroke(false);
  if (!o.disableMultiStroke) {
    ops.push(...stroke(true));
  }
  return ops;
}

if (process.argv[1] && import.meta.url.endsWith(process.argv[1])) {
  console.log(JSON.stringify(roughLineReference(10, 20, 100, 180, 17), null, 2));
}
