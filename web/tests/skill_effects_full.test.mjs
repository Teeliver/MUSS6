import test from 'node:test';
import assert from 'node:assert/strict';
import {skillEffectSpecs, skillEffectSpec, skillEffectFamilies, skillEffectCoverage, legacyExactSkills, masterActiveSkills, isActiveSkill} from '../client/src/world/skill-effect-catalog.js';
import {createEffectPlan, sampleEffectGeometry, effectDuration, cameraSource} from '../client/src/world/skill-effect-runtime.js';
import {skillCatalog} from '../client/src/ui/skill-catalog.js';

const active = skillCatalog.skills.filter(isActiveSkill);

test('mọi skill chủ động của 7 class đều có effect tường minh', () => {
  assert.equal(active.length, 114);
  for (const skill of active) assert.ok(skillEffectSpec(skill.id), 'thiếu effect cho ' + skill.id + ' ' + skill.name);
  const ids = new Set(active.map(s => s.id));
  for (const id of Object.keys(skillEffectSpecs).map(Number)) assert.ok(ids.has(id), 'effect thừa cho id ' + id);
});

test('không tự sinh effect cho skill thụ động hoặc id lạ', () => {
  for (const id of [0, 300, 326, 500, 511, 515, 517, 999, null, undefined, '4']) assert.equal(skillEffectSpec(id), null);
  for (const id of legacyExactSkills) assert.equal(skillEffectSpec(id).exact, true);
  assert.deepEqual([...legacyExactSkills], [4, 9, 17]);
  assert.equal(skillEffectSpec(79), null);
});

test('chỉ 10 master skill có handler server mới được gán effect', () => {
  assert.deepEqual([...masterActiveSkills], [344, 425, 426, 427, 430, 432, 461, 495, 521, 565]);
  for (const id of masterActiveSkills) assert.ok(skillEffectSpec(id), 'thiếu master skill ' + id);
  for (const id of Object.keys(skillEffectSpecs).map(Number).filter(value => value >= 300)) assert.ok(masterActiveSkills.includes(id), 'effect thừa cho master skill ' + id);
});

test('mỗi họ effect trỏ tới texture Data/Effect hợp lệ và không trùng', () => {
  const seen = new Set();
  for (const [name, family] of Object.entries(skillEffectFamilies)) {
    assert.match(family.texture, /^Effect\/[A-Za-z0-9_!-]+\.(?:OZJ|OZT)$/, name);
    assert.ok(Number.isFinite(family.size) && family.size > 0, name);
    assert.equal(seen.has(family.texture), false, 'texture trùng: ' + family.texture);
    seen.add(family.texture);
  }
});

test('mọi mode dựng hình học hữu hạn và giữ nguyên số mesh qua thời gian', () => {
  const modes = new Set(Object.values(skillEffectSpecs).map(s => s.mode));
  assert.ok(modes.size >= 10);
  for (const mode of modes) {
    const id = Object.keys(skillEffectSpecs).find(key => skillEffectSpecs[key].mode === mode);
    const spec = skillEffectSpec(Number(id));
    const plan = createEffectPlan({spec, from: {x: 0, y: 0, z: 1}, to: {x: 3, y: 1, z: 1}, seed: 7});
    assert.ok(Number.isFinite(plan.duration) && plan.duration > 0, mode);
    assert.equal(effectDuration(spec, plan.distance), plan.duration);
    const counts = [];
    for (const age of [0, plan.duration / 2, plan.duration]) {
      const meshes = sampleEffectGeometry(plan, age, [5, 0, 10]);
      counts.push(meshes.length);
      for (const mesh of meshes) {
        assert.ok(mesh.positions instanceof Float32Array && mesh.positions.length % 9 === 0, mode);
        assert.equal(mesh.normals.length, mesh.positions.length, mode);
        assert.equal(mesh.uv.length, mesh.positions.length * 2 / 3, mode);
        for (const value of mesh.positions) assert.ok(Number.isFinite(value), mode);
        for (const value of mesh.uv) assert.ok(Number.isFinite(value), mode);
        assert.ok(Number.isFinite(mesh.opacity) && mesh.opacity >= 0 && mesh.opacity <= 1, mode);
      }
    }
    assert.equal(new Set(counts).size, 1, 'số mesh đổi theo thời gian ở mode ' + mode);
  }
});

test('model effect source cho Earthquake/Force Wave giải mã được', async () => {
  const {readFile} = await import('node:fs/promises');
  const {decodeBmdGeometry} = await import('../client/src/platform/assets/decode-model.js');
  const {core} = await import('./helpers/game-fixture.mjs');
  const root = '/Users/angelale/Downloads/MuClientEX603/Data/';
  for (const [id, path] of [[15, 'Skill/Circle01.bmd'], [62, 'Skill/EarthQuake01.bmd'], [66, 'Skill/m_waves.bmd'], [233, 'Effect/magic_powerup.bmd'], [260, 'Effect/wolf_head_effect.bmd'], [261, 'Effect/down_left_punch.bmd'], [263, 'Effect/wind_foce.bmd'], [265, 'Effect/multishot01.bmd'], [270, 'Effect/multishot01.bmd'], [223, 'Skill/summon_sahamutt.bmd'], [224, 'Skill/summon_neil.bmd'], [225, 'Skill/summon_lagul.bmd'], [235, 'Effect/multishot01.bmd'], [210, 'Skill/eventshild.bmd'], [211, 'Effect/atshild.bmd'], [213, 'Skill/Inferno01.bmd'], [215, 'Effect/lightning_type01.bmd'], [234, 'Skill/nightmaresum.bmd']]) {
    assert.equal(skillEffectSpec(id).model, path);
    const model = decodeBmdGeometry(core, new Uint8Array(await readFile(root + path)));
    assert.ok(model.meshes.length > 0, path);
  }
  assert.equal(skillEffectSpec(215).anchor, 'from', 'Chain Lightning gắn effect vào tay người cast');
});

test('coverage đếm đúng prototype/nguồn/họ và không còn missing', () => {
  const coverage = skillEffectCoverage(active);
  assert.deepEqual(coverage, {total: 114, exact: 3, source: 106, derived: 0, noVisual: 5, missing: 0});
});

test('5 skill không có visual trong PC client được đánh dấu no-visual, không gán effect giả', () => {
  for (const id of [212, 344, 495, 521, 565]) {
    const spec = skillEffectSpec(id);
    assert.equal(spec.noVisual, true, 'skill ' + id);
    assert.equal(spec.family, undefined, 'skill ' + id);
    assert.equal(typeof spec.reason, 'string', 'skill ' + id);
  }
});

test('catalog giữ đúng 7 class và id gốc', () => {
  const classes = new Set(active.flatMap(s => s.classes.map((v, i) => v ? i : -1)).filter(i => i >= 0));
  assert.deepEqual([...classes].sort((a, b) => a - b), [0, 1, 2, 3, 4, 5, 6]);
});
