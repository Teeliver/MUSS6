// Source: GameServer/ChaosBox.cpp (PlusItemLevelMix, ChaosItemMix, Item380Mix, SocketWeaponMix).
// Only weapon/upgrade recipes get a client preflight; other recipes are still validated by server.

const PLUS_LEVEL = new Map([[3, 9], [4, 10], [22, 11], [23, 12], [49, 13], [50, 14]]);

const SOCKET_MATERIAL_NAMES = new Map([
  [14 * 512 + 180, 'Ice Walker Mane'],
  [14 * 512 + 181, 'Mammoth Bundle of Fur'],
  [14 * 512 + 182, 'Giant Icicle'],
  [14 * 512 + 183, 'Coolutin Poison'],
  [14 * 512 + 184, 'Iron Knight Heart'],
  [14 * 512 + 185, 'Dark Mammoth Horn'],
  [14 * 512 + 186, 'Dark Giant Axe'],
  [14 * 512 + 187, 'Dark Coolutin Blood'],
  [14 * 512 + 188, 'Dark Iron Knight Sword']
]);

const SOCKET_WEAPON_RECIPES = new Map([
  [1, [[14 * 512 + 180, 2], [14 * 512 + 188, 1]]],
  [2, [[14 * 512 + 181, 2], [14 * 512 + 185, 1]]],
  [3, [[14 * 512 + 182, 2], [14 * 512 + 188, 1]]],
  [4, [[14 * 512 + 183, 2], [14 * 512 + 185, 1]]],
  [5, [[14 * 512 + 182, 2], [14 * 512 + 186, 1]]],
  [6, [[14 * 512 + 180, 2], [14 * 512 + 185, 1]]],
  [7, [[14 * 512 + 184, 2], [14 * 512 + 187, 1]]],
  [8, [[14 * 512 + 182, 2], [14 * 512 + 185, 1]]],
  [9, [[14 * 512 + 184, 2], [14 * 512 + 186, 1]]],
  [10, [[14 * 512 + 184, 2], [14 * 512 + 188, 1]]],
  [11, [[14 * 512 + 181, 2], [14 * 512 + 187, 1]]],
  [12, [[14 * 512 + 184, 2], [14 * 512 + 185, 1]]]
]);

export const weaponMixRecipes = Object.freeze([1, 3, 4, 22, 23, 36, 49, 50, 57]);

export function isWeaponMix(id) {
  return weaponMixRecipes.includes(id);
}

const row = (label, need, have) => ({
  text: `+${label} ${Math.min(have, need)}/${need}`,
  label, need, have,
  state: have >= need ? 'ok' : have > 0 ? 'partial' : 'none'
});

export function chaosMixRequirements(id, items = []) {
  const list = Array.isArray(items) ? items : [];
  const count = index => list.filter(item => item?.index === index).length;
  const itemsAtLevel = level => list.filter(item => item?.level === level && ![12 * 512 + 15, 14 * 512 + 13, 14 * 512 + 14].includes(item?.index));

  if (PLUS_LEVEL.has(id)) {
    const target = PLUS_LEVEL.get(id), need = target - 8;
    const rows = [
      row('Jewel of Chaos', 1, count(12 * 512 + 15)),
      row('Jewel of Bless', need, count(14 * 512 + 13)),
      row('Jewel of Soul', need, count(14 * 512 + 14)),
      row(`Món +${target}`, 1, itemsAtLevel(target).length)
    ];
    return {rows, ok: rows.every(item => item.have >= item.need), text: ''};
  }

  if (id === 1) {
    const candidates = list.filter(item => item?.level >= 4 && (item?.option ?? 0) >= 1 && ![12 * 512 + 15, 14 * 512 + 13, 14 * 512 + 14].includes(item?.index)).length;
    const rows = [row('Jewel of Chaos', 1, count(12 * 512 + 15)), row('Món +4 có option', 1, candidates)];
    return {rows, ok: rows.every(item => item.have >= item.need), text: ''};
  }

  if (id === 36) {
    const candidates = list.filter(item => item?.level >= 4 && (item?.option ?? 0) >= 1 && !item?.option380).length;
    const rows = [
      row('Jewel of Guardian', 1, count(14 * 512 + 31)),
      row('Jewel of Harmony', 1, count(14 * 512 + 42)),
      row('Món +4 có option, chưa 380', 1, candidates)
    ];
    return {rows, ok: rows.every(item => item.have >= item.need), text: ''};
  }

  if (id === 57) {
    const notes = list.filter(item => item?.index >= 14 * 512 + 191 && item?.index <= 14 * 512 + 202);
    const socketWeapons = list.filter(item => {
      const group = (item?.index ?? 0) >> 9;
      return group >= 0 && group <= 5 && (item?.level ?? 0) >= 7 && (item?.option ?? 0) >= 1;
    });
    const noteRow = row('Combination Note', 1, notes.length === 1 ? 1 : 0);
    noteRow.text = `+Combination Note ${notes.length}/1`;
    const rows = [noteRow, row('Vũ khí socket +7, option +1', 1, socketWeapons.length)];
    if (notes.length >= 1) {
      const type = notes[0].index - (14 * 512 + 190), recipe = SOCKET_WEAPON_RECIPES.get(type);
      if (recipe) for (const [index, need] of recipe) rows.push(row(SOCKET_MATERIAL_NAMES.get(index) ?? ('Material ' + index), need, count(index)));
    }
    return {rows, ok: rows.every(item => item.have >= item.need), text: '', note: notes.length === 1 ? notes[0].index - (14 * 512 + 190) : undefined};
  }

  return {rows: [{text: '+Nguyên liệu theo công thức (server kiểm tra)', state: 'hint', have: 0, need: 1}], ok: true, unknown: true, text: ''};
}

export function validateChaosMix(id, items = []) {
  const result = chaosMixRequirements(id, items);
  if (result.unknown) return {ok: true, text: ''};
  const missing = result.rows.filter(row => row.have < row.need);
  if (missing.length) return {ok: false, text: 'Thiếu: ' + missing.map(row => row.text).join(', ')};
  return {ok: true, text: result.note ? `Hợp lệ: Socket Weapon note ${result.note}.` : ''};
}

// Source-like auto detection (MixMgr.cpp CheckRecipe + CheckRecipeSimilarity).
// The PC client builds the recipe from the items in the Chaos Box instead of
// requiring the player to pick one manually. Exact recipes win first; otherwise
// the most similar known recipe is shown so the player sees what is being built.
const DETECT_ORDER = [3, 4, 22, 23, 49, 50, 36, 57, 1];
const primaryRow = text => /^\+?(Món|Vũ khí|Combination Note)/.test(text);

export function detectChaosMix(recipes, items = []) {
  const known = DETECT_ORDER.filter(id => recipes.includes(id));
  for (const id of known) if (chaosMixRequirements(id, items).ok) return id;
  let best = null, bestScore = 0;
  for (const id of known) {
    const rows = chaosMixRequirements(id, items).rows;
    let score = 0, primary = false;
    for (const item of rows) {
      if (item.have <= 0) continue;
      if (primaryRow(item.text)) { score += 10; primary = true; }
      else score += 1;
    }
    if (primary && score > bestScore) { best = id; bestScore = score; }
  }
  return best;
}
