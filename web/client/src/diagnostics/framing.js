export function checkFraming(module) {
  const expected = [[0xc1,4,0,1],[0xc2,0,5,0xf4,6],[0xc3,4,0,0],[0xc4,0,5,0,0]];
  const stream = Uint8Array.from(expected.flat());
  for (let split = 0; split <= stream.length; split++) {
    const reader = new module.FrameReader();
    try {
      const frames = [...reader.feed(stream.subarray(0,split)), ...reader.feed(stream.subarray(split))];
      if (JSON.stringify(frames.map(frame => [...frame])) !== JSON.stringify(expected)) {
        throw new Error(`Framing mismatch at split ${split}`);
      }
      reader.finish();
    } finally { reader.delete(); }
  }
  return { frames: expected.length, splitCases: stream.length + 1 };
}
