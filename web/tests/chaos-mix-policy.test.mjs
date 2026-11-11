import test from 'node:test';
import assert from 'node:assert/strict';
import {validateChaosMix, chaosMixRequirements, weaponMixRecipes, isWeaponMix, detectChaosMix} from '../client/src/protocol/chaos-mix-policy.js';
import {mixModeRecipes} from '../client/src/protocol/npc-extensions.js';
import {jewelAction, jewelCanUse, JEWEL_OF_BLESS, SMELTING_STONE_LOW, JEWEL_OF_EXTENSION} from '../client/src/ui/jewel-use-policy.js';
import {createWorldSession} from '../client/src/session/world-session.js';

const item = (index, extra = {}) => ({index, level: 0, option: 0, ...extra});

test('weapon mix recipes cover upgrade, 380 and socket weapon', () => {
  assert.deepEqual([...weaponMixRecipes], [1, 3, 4, 22, 23, 36, 49, 50, 57]);
  assert.equal(isWeaponMix(57), true);
  assert.equal(isWeaponMix(42), false);
});

test('Plus Item Level preflight matches ChaosBox.cpp counts', () => {
  const ok = [item(12 * 512 + 15), item(14 * 512 + 13), item(14 * 512 + 14), item(0, {level: 9})];
  assert.equal(validateChaosMix(3, ok).ok, true);
  assert.equal(validateChaosMix(3, ok.slice(0, 3)).ok, false);
  assert.equal(validateChaosMix(50, [item(12 * 512 + 15), ...[1, 2, 3, 4, 5, 6].flatMap(() => [item(14 * 512 + 13), item(14 * 512 + 14)]), item(0, {level: 14})]).ok, true);
});

test('Chaos Weapon and Item 380 preflight', () => {
  assert.equal(validateChaosMix(1, [item(12 * 512 + 15), item(0, {level: 4, option: 1})]).ok, true);
  assert.equal(validateChaosMix(1, [item(12 * 512 + 15), item(0, {level: 3, option: 1})]).ok, false);
  assert.equal(validateChaosMix(36, [item(14 * 512 + 31), item(14 * 512 + 42), item(0, {level: 4, option: 1})]).ok, true);
  assert.equal(validateChaosMix(36, [item(14 * 512 + 31), item(14 * 512 + 42), item(0, {level: 4, option: 1, option380: true})]).ok, false);
});

test('Socket Weapon preflight resolves note material table', () => {
  const base = [item(14 * 512 + 191), item(14 * 512 + 180), item(14 * 512 + 180), item(14 * 512 + 188), item(0, {level: 7, option: 1, socketCount: 1})];
  assert.equal(validateChaosMix(57, base).ok, true);
  assert.match(validateChaosMix(57, base).text, /note 1/);
  assert.equal(validateChaosMix(57, base.filter(x => x.index !== 14 * 512 + 188)).ok, false);
  assert.equal(validateChaosMix(57, [...base, item(14 * 512 + 192)]).ok, false);
});

test('ingredient rows mark red/partial/gold and fee only when complete', () => {
  assert.equal(chaosMixRequirements(3, []).rows.every(row => row.state === 'none'), true);
  const partial = chaosMixRequirements(4, [item(12 * 512 + 15), item(14 * 512 + 13), item(0, {level: 10})]);
  assert.equal(partial.rows.find(row => row.label === 'Jewel of Bless').state, 'partial');
  assert.equal(partial.ok, false);
  const full = chaosMixRequirements(3, [item(12 * 512 + 15), item(14 * 512 + 13), item(14 * 512 + 14), item(0, {level: 9})]);
  assert.equal(full.rows.every(row => row.state === 'ok'), true);
  assert.equal(full.ok, true);
});

test('auto detection follows MixMgr exact then most-similar behaviour', () => {
  const recipes = [...weaponMixRecipes];
  assert.equal(detectChaosMix(recipes, []), null);
  assert.equal(detectChaosMix(recipes, [item(12 * 512 + 15), item(14 * 512 + 13), item(14 * 512 + 14), item(0, {level: 9})]), 3);
  assert.equal(detectChaosMix(recipes, [item(0, {level: 10})]), 4);
  assert.equal(detectChaosMix(recipes, [item(12 * 512 + 15), item(0, {level: 5, option: 1})]), 1);
  assert.equal(detectChaosMix(recipes, [item(14 * 512 + 31), item(14 * 512 + 42), item(0, {level: 4, option: 1})]), 36);
  assert.equal(detectChaosMix(recipes, [item(14 * 512 + 191)]), 57);
  assert.match(chaosMixRequirements(3, []).rows[0].text, /^\+/);
});

test('direct jewel preflight includes smelting/extension and item restrictions', () => {
  assert.equal(jewelAction(SMELTING_STONE_LOW), 'smelt1');
  assert.equal(jewelAction(JEWEL_OF_EXTENSION), 'extension');
  assert.equal(jewelCanUse('bless', {index: 0, level: 5}), true);
  assert.equal(jewelCanUse('bless', {index: 0, level: 6}), false);
  assert.equal(jewelCanUse('soul', {index: 12 * 512 + 7, level: 0}), false);
  assert.equal(jewelCanUse('soul', {index: 12 * 512 + 38, level: 0}), true);
  assert.equal(jewelCanUse('bless', {index: JEWEL_OF_BLESS, level: 0, periodic: true}), false);
});


test('Chaos Goblin method menu splits regular, chaos weapon and option 380', () => {
  const all = [1, 2, 3, 36, 57];
  assert.deepEqual(mixModeRecipes(3, all, 0), [2, 3, 57]);
  assert.deepEqual(mixModeRecipes(3, all, 1), [1]);
  assert.deepEqual(mixModeRecipes(3, all, 2), [36]);
  assert.deepEqual(mixModeRecipes(7, [13, 14], 0), [13, 14]);
});

test('mixRate is non-blocking: no businessRequest/pending lock on chaos box', async () => {
  const sent = [];
  const world = createWorldSession({timers: {setTimeout: () => 1, clearTimeout() {}, clearInterval() {}}});
  world.attach({greeting: {index: 1}, send: bytes => sent.push([...bytes]), close() {}});
  const entry = new Uint8Array(136);
  entry.set([193, 136, 243, 3, 100, 100, 0, 3]);
  new DataView(entry.buffer).setUint32(80, 100, true);
  const ready = world.select({name: 'Mixer'}, 0);
  world.receive(entry);
  await ready;
  const wire = [0, 0, 20, 0, 0, 0, 0, 255, 255, 255, 255, 255];
  world.receive(Uint8Array.of(194, 0, 19, 243, 16, 1, 12, ...wire));
  world.receive(Uint8Array.of(194, 0, 15, 19, 1, 1, 2, 0, 251, 102, 100, 102, 100, 0, 0));
  const opening = world.talkNpc(258);
  world.receive(Uint8Array.of(195, 11, 48, 3, 0, 0, 0, 0, 0, 0, 0));
  await opening;
  world.receive(Uint8Array.of(193, 6, 178, 26, 1, 3));
  assert.equal(world.gameState.npcShop.chaosTax, 3);
  assert.equal(world.gameState.npcShop.tax, undefined);
  assert.equal(world.gameState.npcShop.mixMenu, undefined);
  assert.equal(world.setMixMode(2), true);
  assert.equal(world.gameState.npcShop.mixMode, 2);
  assert.equal(world.setMixMode(0), true);
  world.receive(Uint8Array.of(194, 0, 6, 49, 3, 0));
  assert.equal(world.mixRate(1), true);
  assert.deepEqual(sent.at(-1), [193, 8, 136, 0, 1, 0, 0, 0]);
  const move = world.npcMoveItem(0, 12, 3, 0);
  assert.deepEqual(sent.at(-1).slice(0, 5), [195, 19, 36, 0, 12]);
  world.receive(Uint8Array.of(195, 17, 36, 3, 0, ...wire));
  assert.equal(await move, true);
  assert.equal(world.gameState.npcShop.items[0].slot, 0);
  world.receive(Uint8Array.of(194, 0, 6, 49, 3, 0));
  assert.equal(world.closeNpc(), true);
  assert.deepEqual(sent.at(-1), [193, 3, 135]);
  world.close();
});
