// Orbs/scrolls have durability 0 in Item_eng.bmd but are still usable.
// Mirrors GameServer CGItemUseRecv: group 12 orbs/scrolls + group 15 ETC.
export function useWithoutDurability(index) {
  if (!Number.isInteger(index) || index < 0) return false;
  const group = index >> 9;
  const type = index & 511;
  if (group === 12) return (type >= 7 && type <= 24 && type !== 15) || type === 35 || (type >= 44 && type <= 48);
  return group === 15;
}
