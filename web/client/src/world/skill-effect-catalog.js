// W0115 — catalog effect cho toàn bộ skill chủ động của 7 class.
// Nguồn ánh xạ: Main5.2/source/ZzzCharacter.cpp (dispatch `switch (c->Skill)`),
// Main5.2/source/SkillManager.h (AT_SKILL_* / Skill_* id) và Data/Effect.
// `source` chỉ ghi khi tìm thấy CreateEffect/CreateJoint/CreateParticle thật.
// `derived` = cùng họ hình ảnh với skill gốc của class, chưa có token nguồn riêng.
// Effect không bao giờ được suy từ tên skill: mọi dòng đều phải có family tường minh.

export const skillEffectFamilies = {
  fire: {texture: 'Effect/firered.OZJ', size: 1.3},
  ice: {texture: 'Effect/snowseff01.OZJ', size: 1.1},
  lightning: {texture: 'Effect/lightning.OZJ', size: 1.05},
  energy: {texture: 'Effect/energy01.OZJ', size: 1.2},
  poison: {texture: 'Effect/ghosteffect01.OZJ', size: 1.35},
  holy: {texture: 'Effect/shiny04.OZJ', size: 1.15},
  dark: {texture: 'Effect/magicmono_R.OZJ', size: 1.25},
  wind: {texture: 'Effect/wind01.OZJ', size: 1.45},
  water: {texture: 'Effect/water.OZJ', size: 1.2},
  arc: {texture: 'Effect/sword_blur.OZJ', size: 1.55},
  magic: {texture: 'Effect/Magic_Circle1.OZJ', size: 2.0},
  spark: {texture: 'Effect/spark.OZJ', size: 0.9},
  force: {texture: 'Effect/energy02.OZJ', size: 1.1},
  magic2: {texture: 'Effect/Magic_Ground2.OZJ', size: 1.5},
  spark3: {texture: 'Effect/Spark03.OZJ', size: 1.25},
  lightning2: {texture: 'Effect/lightning2.OZJ', size: 1.2},
  swordeff: {texture: 'Effect/!SwordEff.OZJ', size: 1.4},
  swordeffmono: {texture: 'Effect/SwordEff_mono.OZJ', size: 1.4},
  lava: {texture: 'Effect/Lava.OZJ', size: 1.5},
  shiny05: {texture: 'Effect/shiny05.OZJ', size: 1.2},
  nskill: {texture: 'Effect/NSkill.OZJ', size: 1.3},
  shockwave: {texture: 'Effect/ShockWave.OZJ', size: 1.4},
  inferno: {texture: 'Effect/Inferno.OZJ', size: 1.5},
  impack: {texture: 'Effect/Impack03.OZJ', size: 1.2}
};

const spec = (family, mode, source = null, extra = {}) => Object.freeze({family, mode, source, ...extra});

// 114 skill chủ động cast được:
// - id < 300: mana > 0 hoặc AG > 0, class != 0;
// - id >= 300: chỉ 10 skill có handler thật trong CSkillManager::RunningSkill
//   (GameServer/SkillManager.cpp). Các master-skill passive như 511/515/517
//   không nằm trong switch nên không được gán effect.
export const masterActiveSkills = Object.freeze([344, 425, 426, 427, 430, 432, 461, 495, 521, 565]);

export function isActiveSkill(skill) {
  if (!skill || !Array.isArray(skill.classes) || !skill.classes.some(Boolean)) return false;
  if (!Number.isInteger(skill.id) || skill.id < 1) return false;
  if (skill.id >= 300) return masterActiveSkills.includes(skill.id);
  return skill.mana > 0 || skill.ag > 0;
}

export const skillEffectSpecs = {
  1: spec('poison', 'projectile', 'AT_SKILL_POISON'),
  2: spec('fire', 'meteor', 'AT_SKILL_METEO'),
  3: spec('lightning2', 'bolt', 'BITMAP_LIGHTNING + 1'),
  4: spec('fire', 'projectile', 'AT_SKILL_FIREBALL', {exact: true}),
  5: spec('fire', 'column', 'AT_SKILL_FLAME'),
  6: spec('spark3', 'burst', 'BITMAP_SPARK + 1'),
  7: spec('ice', 'projectile', 'AT_SKILL_SLOW'),
  8: spec('wind', 'ring', 'AT_SKILL_STORM'),
  9: spec('dark', 'aura', 'Skill_Evil_Spirit', {exact: true}),
  10: spec('fire', 'column', 'AT_SKILL_HELL'),
  11: spec('force', 'beam', 'AT_SKILL_POWERWAVE'),
  12: spec('water', 'beam', 'AT_SKILL_FLASH'),
  13: spec('fire', 'meteor', 'Skill_Cometfall'),
  14: spec('fire', 'column', 'AT_SKILL_INFERNO'),
  15: spec('magic', 'circle', 'MODEL_CIRCLE', {model: 'Skill/Circle01.bmd', modelScale: .01, duration: 1.2}),
  16: spec('magic2', 'shield', 'AT_SKILL_WIZARDDEFENSE'),
  17: spec('energy', 'projectile', 'AT_SKILL_ENERGYBALL', {exact: true}),
  18: spec('magic2', 'aura', 'BITMAP_MAGIC + 1'),
  19: spec('spark3', 'arc', 'CreateSpark'),
  20: spec('spark3', 'arc', 'CreateSpark'),
  21: spec('spark3', 'arc', 'CreateSpark'),
  22: spec('spark3', 'arc', 'CreateSpark'),
  23: spec('spark3', 'arc', 'CreateSpark'),
  24: spec('spark', 'projectile', 'CreateArrows'),
  26: spec('magic2', 'aura', 'AT_SKILL_HEALING'),
  27: spec('magic2', 'shield', 'AT_SKILL_DEFENSE'),
  28: spec('magic2', 'aura', 'AT_SKILL_ATTACK'),
  30: spec('magic', 'circle', 'AT_SKILL_SUMMON'),
  31: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  32: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  33: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  34: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  35: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  36: spec('magic2', 'circle', 'BITMAP_MAGIC + 1'),
  38: spec('poison', 'projectile', 'AT_SKILL_BLAST_POISON'),
  39: spec('ice', 'ring', 'AT_SKILL_BLAST_FREEZE'),
  40: spec('lightning', 'ring', 'AT_SKILL_BLAST_HELL'),
  41: spec('wind', 'ring', 'Skill_Twisting_Slash'),
  42: spec('arc', 'burst', 'AT_SKILL_FURY_STRIKE'),
  43: spec('dark', 'arc', 'Skill_Death_Stab'),
  44: spec('arc', 'arc', 'AT_SKILL_RUSH'),
  45: spec('force', 'beam', 'AT_SKILL_JAVELIN'),
  46: spec('spark', 'projectile', 'CreateArrows'),
  47: spec('force', 'projectile', 'Skill_Impale'),
  48: spec('holy', 'aura', 'AT_SKILL_VITALITY'),
  49: spec('fire', 'beam', 'AT_SKILL_RIDER'),
  51: spec('ice', 'projectile', 'AT_SKILL_PARALYZE'),
  52: spec('spark', 'beam', 'AT_SKILL_PIERCING'),
  55: spec('fire', 'arc', 'Skill_Fire_Slash'),
  56: spec('ice', 'arc', 'AT_SKILL_ICE_BLADE'),
  57: spec('wind', 'arc', 'AT_SKILL_ONEFLASH'),
  60: spec('force', 'projectile', 'AT_SKILL_STRONG_PIER'),
  61: spec('fire', 'burst', 'Skill_Fire_Burst'),
  62: spec('force', 'ring', 'MODEL_SKILL_FURY_STRIKE', {model: 'Skill/EarthQuake01.bmd', modelScale: .01, duration: 1.1}),
  63: spec('magic', 'circle', 'AT_SKILL_PARTY_TELEPORT'),
  64: spec('holy', 'aura', 'AT_SKILL_ADD_CRITICAL'),
  65: spec('nskill', 'burst', 'AT_SKILL_THUNDER_STRIKE'),
  66: spec('force', 'ring', 'MODEL_WAVES', {model: 'Skill/m_waves.bmd', modelScale: .01, duration: .8}),
  67: spec('lightning', 'burst', 'AT_SKILL_STUN'),
  68: spec('holy', 'aura', 'AT_SKILL_REMOVAL_STUN'),
  69: spec('holy', 'aura', 'AT_SKILL_MANA'),
  70: spec('magic', 'aura', 'AT_SKILL_INVISIBLE'),
  71: spec('holy', 'aura', 'AT_SKILL_REMOVAL_INVISIBLE'),
  72: spec('holy', 'aura', 'AT_SKILL_REMOVAL_BUFF'),
  73: spec('energy', 'beam', 'AT_SKILL_DEATH_CANNON'),
  74: spec('fire', 'burst', 'Skill_Fire_Blast'),
  75: spec('dark', 'burst', 'AT_SKILL_BRAND_OF_SKILL'),
  76: spec('lightning', 'bolt', 'AT_SKILL_PLASMA_STORM_FENRIR'),
  77: spec('spark', 'projectile', 'MODEL_INFINITY_ARROW'),
  78: spec('fire', 'burst', 'Skill_Fire_Scream'),
  210: spec('shockwave', 'shield', 'MODEL_CURSEDTEMPLE_PRODECTION_SKILL', {model: 'Skill/eventshild.bmd', modelScale: .01, duration: 1.2}),
  211: spec('shockwave', 'ring', 'MODEL_SHIELD_CRASH + BITMAP_SHOCK_WAVE', {model: 'Effect/atshild.bmd', modelScale: .01, duration: .9}),
  213: spec('inferno', 'burst', 'MODEL_SKILL_INFERNO + MODEL_SHIELD_CRASH', {model: 'Skill/Inferno01.bmd', modelScale: .01, duration: 1.2}),
  214: spec('dark', 'beam', 'AT_SKILL_ALICE_DRAINLIFE'),
  215: spec('lightning2', 'chain', 'MODEL_FENRIR_THUNDER + BITMAP_LIGHT', {model: 'Effect/lightning_type01.bmd', modelScale: .01, duration: .8, anchor: 'from'}),
  216: spec('lightning', 'bolt', 'AT_SKILL_ALICE_LIGHTNINGORB'),
  217: spec('holy', 'shield', 'AT_SKILL_ALICE_THORNS'),
  218: spec('fire', 'aura', 'AT_SKILL_ALICE_BERSERKER'),
  219: spec('dark', 'aura', 'AT_SKILL_ALICE_SLEEP'),
  221: spec('poison', 'aura', 'AT_SKILL_ALICE_WEAKNESS'),
  222: spec('dark', 'aura', 'AT_SKILL_ALICE_ENERVATION'),
  223: spec('fire', 'burst', 'MODEL_SUMMONER_SUMMON_SAHAMUTT', {model: 'Skill/summon_sahamutt.bmd', modelScale: .01, duration: 1}),
  224: spec('dark', 'ring', 'MODEL_SUMMONER_SUMMON_NEIL', {model: 'Skill/summon_neil.bmd', modelScale: .01, duration: 1}),
  225: spec('poison', 'ring', 'MODEL_SUMMONER_SUMMON_LAGUL', {model: 'Skill/summon_lagul.bmd', modelScale: .01, duration: 1}),
  230: spec('lightning', 'bolt', 'AT_SKILL_LIGHTNING_SHOCK'),
  232: spec('dark', 'burst', 'AT_SKILL_BLOW_OF_DESTRUCTION'),
  233: spec('magic2', 'aura', 'MODEL_SWELL_OF_MAGICPOWER', {model: 'Effect/magic_powerup.bmd', modelScale: .01, duration: 1}),
  234: spec('impack', 'aura', 'MODEL_SUMMON + BITMAP_IMPACT', {model: 'Skill/nightmaresum.bmd', modelScale: .01, duration: 1}),
  235: spec('spark', 'projectile', 'MODEL_MULTI_SHOT1', {model: 'Effect/multishot01.bmd', modelScale: .01, duration: .8}),
  236: spec('fire', 'arc', 'AT_SKILL_FLAME_STRIKE'),
  237: spec('wind', 'ring', 'AT_SKILL_GIGANTIC_STORM'),
  238: spec('shiny05', 'burst', 'BITMAP_SHINY + 6'),
  260: spec('dark', 'burst', 'MODEL_WOLF_HEAD_EFFECT', {model: 'Effect/wolf_head_effect.bmd', modelScale: .01, duration: .8}),
  261: spec('force', 'burst', 'MODEL_DOWN_ATTACK_DUMMY_R', {model: 'Effect/down_left_punch.bmd', modelScale: .01, duration: .8}),
  262: spec('swordeff', 'arc', 'BITMAP_SWORDEFF'),
  263: spec('wind', 'ring', 'MODEL_WINDFOCE', {model: 'Effect/wind_foce.bmd', modelScale: .01, duration: .9}),
  264: spec('lava', 'burst', 'BITMAP_LAVA'),
  265: spec('spark', 'burst', 'MODEL_MULTI_SHOT1', {model: 'Effect/multishot01.bmd', modelScale: .01, duration: .9}),
  266: spec('swordeffmono', 'arc', 'BITMAP_SWORD_EFFECT_MONO'),
  267: spec('swordeffmono', 'aura', 'BITMAP_SWORD_EFFECT_MONO'),
  268: spec('swordeffmono', 'shield', 'BITMAP_SWORD_EFFECT_MONO'),
  269: spec('wind', 'burst', 'AT_SKILL_OCCUPY'),
  270: spec('spark', 'burst', 'MODEL_MULTI_SHOT1', {model: 'Effect/multishot01.bmd', modelScale: .01, duration: .9}),
  425: spec('magic2', 'aura', 'Skill_Cure'),
  426: spec('magic2', 'aura', 'Skill_Party_Healing'),
  427: spec('poison', 'projectile', 'CreateArrows'),
  430: spec('magic2', 'aura', 'Skill_Bless'),
  432: spec('magic2', 'circle', 'Skill_Summon_Satyros'),
  461: spec('magic2', 'aura', 'AT_SKILL_ALICE_BLIND')
};

// Skill chủ động nhưng PC client nguồn không có cast visual:
// không gán texture/model để tránh bịa effect.
export const noVisualSkills = Object.freeze({
  212: 'AT_SKILL_CURSED_TEMPLE_TELEPORT chỉ SetAction(PLAYER_HIT_REMOVAL), không có Create* trong client',
  344: 'Skill_Blood_Storm chỉ có enum + xử lý server, không có case/Create* trong client',
  495: 'Skill_Earth_Prison chỉ có buff icon EFFECT_EARTH_PRISON, không có cast effect client',
  521: 'Skill_Iron_Defense chỉ có buff icon EFFECT_IRON_DEFENSE, không có cast effect client',
  565: 'Skill_Blood_Howling chỉ có buff icon EFFECT_BLOOD_HOWLING, không có cast effect client'
});

export const legacyExactSkills = Object.freeze([4, 9, 17]);

export function skillEffectSpec(id) {
  if (!Number.isInteger(id) || id < 1) return null;
  if (noVisualSkills[id]) return Object.freeze({id, noVisual: true, reason: noVisualSkills[id]});
  const entry = skillEffectSpecs[id];
  if (!entry) return null;
  const family = skillEffectFamilies[entry.family];
  if (!family) throw new Error('Unknown skill effect family: ' + entry.family);
  return Object.freeze({...entry, id, texture: family.texture, size: entry.size ?? family.size});
}

export function skillEffectCoverage(rows) {
  const result = {total: 0, exact: 0, source: 0, derived: 0, noVisual: 0, missing: 0};
  for (const row of rows) {
    const id = typeof row === 'number' ? row : (row?.index ?? row?.id);
    const entry = skillEffectSpec(id);
    result.total++;
    if (!entry) result.missing++;
    else if (entry.noVisual) result.noVisual++;
    else if (entry.exact) result.exact++;
    else if (entry.source) result.source++;
    else result.derived++;
  }
  return result;
}
