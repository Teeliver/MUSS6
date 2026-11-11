import {skillEffectSpec} from '../world/skill-effect-catalog.js';

// Chỉ 3 prototype gốc có fixture preview riêng (fire/energy/spirit).
// Mọi skill chủ động khác báo đúng họ effect đã đăng ký, không giả lập parity.
const prototypes = new Map([
  [9, {effect: 'spirit', label: 'Evil Spirit thử'}],
  [4, {effect: 'fire', label: 'Fire01 thử'}],
  [17, {effect: 'energy', label: 'Energy thử'}]
]);

export function skillSupport(index) {
  const prototype = prototypes.get(index);
  if (prototype) return {...prototype, status: 'prototype', family: prototype.effect};
  const spec = skillEffectSpec(index);
  if (!spec) return {effect: null, label: 'không có effect khi cast', status: 'missing', family: null};
  if (spec.noVisual) return {effect: null, label: `no-visual · ${spec.reason}`, status: 'no-visual', family: null};
  return {
    effect: null,
    label: `${spec.family}/${spec.mode}${spec.source ? ' · nguồn' : ' · họ'}`,
    status: spec.source ? 'source' : 'derived',
    family: spec.family
  };
}

export function skillCoverage(rows) {
  const result = {total: rows.length, prototype: 0, source: 0, derived: 0, noVisual: 0, missing: 0};
  for (const row of rows) {
    const status = skillSupport(row.index).status;
    result[status === 'no-visual' ? 'noVisual' : status]++;
  }
  return result;
}
