import test from 'node:test';
import assert from 'node:assert/strict';
import {skillSupport, skillCoverage} from '../client/src/preview/skill-support.js';

test('ba prototype gốc giữ nhãn preview riêng', () => {
  assert.equal(skillSupport(4).effect, 'fire');
  assert.equal(skillSupport(17).effect, 'energy');
  assert.equal(skillSupport(9).effect, 'spirit');
  for (const id of [4, 9, 17]) assert.equal(skillSupport(id).status, 'prototype');
});

test('skill chủ động khác báo đúng nguồn hoặc họ, không giả lập fixture', () => {
  assert.equal(skillSupport(1).status, 'source');
  assert.equal(skillSupport(1).effect, null);
  assert.equal(skillSupport(3).status, 'source');
  assert.equal(skillSupport(260).status, 'source');
  assert.equal(skillSupport(425).status, 'source');
  assert.equal(skillSupport(344).status, 'no-visual');
  assert.equal(skillSupport(212).status, 'no-visual');
  assert.equal(skillSupport(495).status, 'no-visual');
  assert.equal(skillSupport(521).status, 'no-visual');
  assert.equal(skillSupport(565).status, 'no-visual');
  assert.equal(skillSupport(511).status, 'missing');
  for (const id of [0, 300, 326, 999, null]) assert.equal(skillSupport(id).status, 'missing');
});

test('coverage tách prototype, nguồn, họ và missing', () => {
  assert.deepEqual(skillCoverage([{index: 4}, {index: 17}, {index: 9}]), {total: 3, prototype: 3, source: 0, derived: 0, noVisual: 0, missing: 0});
  assert.deepEqual(skillCoverage([{index: 1}, {index: 3}, {index: 5}, {index: 300}]), {total: 4, prototype: 0, source: 3, derived: 0, noVisual: 0, missing: 1});
  assert.deepEqual(skillCoverage([{index: 344}, {index: 495}, {index: 521}, {index: 565}, {index: 212}]), {total: 5, prototype: 0, source: 0, derived: 0, noVisual: 5, missing: 0});
});

test('trả bản sao, không cho sửa registry', () => {
  const copy = skillSupport(4);
  copy.effect = 'changed';
  assert.equal(skillSupport(4).effect, 'fire');
  const family = skillSupport(2);
  family.family = 'changed';
  assert.equal(skillSupport(2).family, 'fire');
});
