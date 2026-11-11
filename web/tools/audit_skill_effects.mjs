#!/usr/bin/env node
// W0115 — audit texture của catalog effect skill. Chỉ đọc Data, không ghi.
// Dùng: node tools/audit_skill_effects.mjs /duong/dan/Data
import {readFile, access} from 'node:fs/promises';
import createCore from '../.build/wasm/frame-reader.mjs';
import {inspectTexture} from '../client/src/platform/assets/decode-texture.js';
import {skillEffectFamilies, skillEffectSpecs, noVisualSkills} from '../client/src/world/skill-effect-catalog.js';

const root = process.argv[2];
if (!root) throw new Error('Thiếu đường dẫn Data gốc');
const core = await createCore();
const rows = [];
for (const [name, family] of Object.entries(skillEffectFamilies)) {
  const path = `${root}/${family.texture}`;
  await access(path);
  const bytes = new Uint8Array(await readFile(path));
  const type = family.texture.toUpperCase().endsWith('.OZT') ? 'OZT' : 'OZJ';
  const image = inspectTexture(core, type, bytes);
  rows.push({family: name, texture: family.texture, bytes: bytes.length, width: image.width, height: image.height});
}
const used = new Set(Object.values(skillEffectSpecs).map(spec => spec.family));
for (const name of used) if (!skillEffectFamilies[name]) throw new Error('Family thiếu texture: ' + name);
console.log(JSON.stringify({families: rows.length, skills: Object.keys(skillEffectSpecs).length + Object.keys(noVisualSkills).length, rows}, null, 1));
