import test from 'node:test';
import assert from 'node:assert/strict';
import {useWithoutDurability} from '../client/src/ui/item-use-policy.js';
import {createWorldSession} from '../client/src/session/world-session.js';

test('Orb/scroll dùng được dù Item_eng.bmd ghi durability 0', () => {
  for (const index of [12 * 512 + 7, 12 * 512 + 8, 12 * 512 + 14, 12 * 512 + 16, 12 * 512 + 24, 12 * 512 + 35, 12 * 512 + 44, 12 * 512 + 48]) assert.equal(useWithoutDurability(index), true, String(index));
  assert.equal(useWithoutDurability(12 * 512 + 15), false, 'Jewel of Chaos không đi qua use');
  assert.equal(useWithoutDurability(14 * 512 + 0), false, 'potion vẫn cần durability');
  assert.equal(useWithoutDurability(15 * 512 + 0), true, 'scroll/book group 15');
});

test('right-click orb gửi C1:26 use dù durability 0', async () => {
  let clock = 1000;
  const sent = [];
  const world = createWorldSession({now: () => clock, timers: {setTimeout: () => 1, clearTimeout() {}, clearInterval() {}}});
  world.attach({greeting: {index: 1}, send: packet => sent.push([...packet]), close() {}});
  const ready = world.select({name: 'OrbTester'}, 1);
  const entry = new Uint8Array(136);
  entry.set([193, 136, 243, 3, 100, 100, 0, 3]);
  new DataView(entry.buffer).setUint16(132, 1, true);
  new DataView(entry.buffer).setUint32(80, 100, true);
  world.receive(entry);
  await ready;
  const inventory = Uint8Array.of(194, 0, 19, 243, 16, 1, 12, 0x07, 0, 0, 0, 0, 0xC0, 0, 255, 255, 255, 255, 255);
  world.receive(inventory);
  assert.equal(world.gameState.items.find(item => item.slot === 12).index, 12 * 512 + 7);
  clock += 1000;
  assert.equal(world.useItem(12), true);
  assert.deepEqual(sent.at(-1), [195, 6, 38, 12, 0, 0]);
  world.close();
});
