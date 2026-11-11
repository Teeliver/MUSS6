// W0115 — runtime hình học cho effect skill.
// Thuần dữ liệu: không DOM, không WebGL, để test được bằng node.
// Không gian toạ độ khớp với model-preview world shader:
//   source.x = tile X, source.y = tile Y, source.z = độ cao.
// Shader đổi source -> world bằng (x, z, -y), nên "up" của thế giới là trục z.

const TAU = Math.PI * 2;

function mulberry32(seed) {
  let a = seed >>> 0;
  return () => {
    a = (a + 0x6d2b79f5) >>> 0;
    let t = a;
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));
const lerp = (a, b, t) => a + (b - a) * t;

export function cameraSource({entry, terrain, focus}) {
  const dx = focus.x - entry.x, dy = focus.y - entry.y;
  const height = terrain.heightAt(focus.x + 0.5, focus.y + 0.5) - terrain.height(entry.x, entry.y);
  return [5.82 + dx, dy - 4.82, height + 8.5];
}

function basis(camera, center) {
  let vx = center[0] - camera[0], vy = center[1] - camera[1], vz = center[2] - camera[2];
  const length = Math.hypot(vx, vy, vz) || 1;
  vx /= length; vy /= length; vz /= length;
  let rx = -vy, ry = vx, rz = 0;
  const rightLength = Math.hypot(rx, ry, rz) || 1;
  rx /= rightLength; ry /= rightLength; rz /= rightLength;
  const ux = vy * rz - vz * ry;
  const uy = vz * rx - vx * rz;
  const uz = vx * ry - vy * rx;
  return {view: [vx, vy, vz], right: [rx, ry, rz], up: [ux, uy, uz]};
}

function quad(camera, center, width, height, opacity, rotation = 0, image) {
  const {right, up, view} = basis(camera, center);
  let rx = right[0], ry = right[1], rz = right[2];
  let ux = up[0], uy = up[1], uz = up[2];
  if (rotation) {
    const c = Math.cos(rotation), s = Math.sin(rotation);
    const nrx = rx * c + ux * s, nry = ry * c + uy * s, nrz = rz * c + uz * s;
    const nux = ux * c - rx * s, nuy = uy * c - ry * s, nuz = uz * c - rz * s;
    rx = nrx; ry = nry; rz = nrz; ux = nux; uy = nuy; uz = nuz;
  }
  const hw = width / 2, hh = height / 2;
  const positions = new Float32Array(18);
  const normals = new Float32Array(18);
  const uv = Float32Array.of(0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0);
  const corners = [[-1, 1], [1, 1], [1, -1], [-1, 1], [1, -1], [-1, -1]];
  for (let i = 0; i < 6; i++) {
    const [sx, sy] = corners[i];
    positions[i * 3] = center[0] + rx * hw * sx + ux * hh * sy;
    positions[i * 3 + 1] = center[1] + ry * hw * sx + uy * hh * sy;
    positions[i * 3 + 2] = center[2] + rz * hw * sx + uz * hh * sy;
    normals[i * 3] = view[0]; normals[i * 3 + 1] = view[1]; normals[i * 3 + 2] = view[2];
  }
  return {positions, normals, uv, opacity, image, additive: true};
}

function disc(camera, center, radius, opacity, rotation = 0, image) {
  const segments = 12;
  const positions = new Float32Array(segments * 9);
  const normals = new Float32Array(segments * 9).fill(1);
  const uv = new Float32Array(segments * 6);
  for (let i = 0; i < segments; i++) {
    const a0 = rotation + i * TAU / segments, a1 = rotation + (i + 1) * TAU / segments;
    const p0 = [center[0] + Math.cos(a0) * radius, center[1] + Math.sin(a0) * radius, center[2]];
    const p1 = [center[0] + Math.cos(a1) * radius, center[1] + Math.sin(a1) * radius, center[2]];
    const tri = [center, p0, p1];
    const uvs = [[0.5, 0.5], [0.5 + 0.5 * Math.cos(a0), 0.5 + 0.5 * Math.sin(a0)], [0.5 + 0.5 * Math.cos(a1), 0.5 + 0.5 * Math.sin(a1)]];
    for (let v = 0; v < 3; v++) {
      positions.set(tri[v], (i * 3 + v) * 3);
      uv.set(uvs[v], (i * 3 + v) * 2);
    }
  }
  return {positions, normals, uv, opacity, image, additive: true};
}

function ribbon(camera, a, b, width, opacity, image) {
  const mid = [(a[0] + b[0]) / 2, (a[1] + b[1]) / 2, (a[2] + b[2]) / 2];
  const {view} = basis(camera, mid);
  let dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
  const length = Math.hypot(dx, dy, dz) || 1;
  dx /= length; dy /= length; dz /= length;
  let sx = dy * view[2] - dz * view[1];
  let sy = dz * view[0] - dx * view[2];
  let sz = dx * view[1] - dy * view[0];
  const sideLength = Math.hypot(sx, sy, sz) || 1;
  sx = sx / sideLength * width / 2; sy = sy / sideLength * width / 2; sz = sz / sideLength * width / 2;
  const positions = Float32Array.of(
    a[0] - sx, a[1] - sy, a[2] - sz, a[0] + sx, a[1] + sy, a[2] + sz, b[0] + sx, b[1] + sy, b[2] + sz,
    a[0] - sx, a[1] - sy, a[2] - sz, b[0] + sx, b[1] + sy, b[2] + sz, b[0] - sx, b[1] - sy, b[2] - sz
  );
  const normals = new Float32Array(18);
  for (let i = 0; i < 6; i++) normals.set(view, i * 3);
  return {positions, normals, uv: Float32Array.of(0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0), opacity, image, additive: true};
}

function pulse(t) {
  return Math.min(1, t / 0.1) * Math.max(0, 1 - Math.max(0, t - 0.62) / 0.38);
}

const durations = {
  projectile: distance => clamp(distance / 15, 0.22, 1.0),
  meteor: () => 0.55,
  column: () => 0.9,
  ring: () => 0.7,
  burst: () => 0.62,
  bolt: () => 0.5,
  arc: () => 0.45,
  aura: () => 1.2,
  beam: () => 0.7,
  circle: () => 1.0,
  chain: () => 0.6,
  shield: () => 1.2
};

export function effectDuration(spec, distance) {
  const fn = durations[spec.mode];
  if (!fn) throw new Error('Unknown skill effect mode: ' + spec.mode);
  return fn(Math.max(0, distance));
}

export function createEffectPlan({spec, from, to, seed = 1}) {
  const random = mulberry32((seed * 2654435761) >>> 0 || 1);
  const distance = Math.hypot(to.x - from.x, to.y - from.y);
  const duration = effectDuration(spec, distance);
  const base = {
    spec, from: {...from}, to: {...to}, distance, duration, seed,
    angles: Array.from({length: 12}, () => random() * TAU),
    speeds: Array.from({length: 12}, () => 0.6 + random() * 0.9)
  };
  return base;
}

export function sampleEffectGeometry(plan, age, camera) {
  const {spec, from, to, duration} = plan;
  const size = spec.size ?? 1;
  const t = clamp(age / duration, 0, 1);
  const fade = pulse(t);
  const out = [];
  const pushQuad = (center, w, h, opacity, rotation = 0) =>
    out.push(quad(camera, center, w, h, opacity, rotation));

  switch (spec.mode) {
    case 'projectile': {
      const arc = Math.sin(Math.PI * t) * 0.18;
      const head = [lerp(from.x, to.x, t), lerp(from.y, to.y, t), lerp(from.z, to.z, t) + 1.05 + arc];
      pushQuad(head, size * (0.8 + 0.35 * Math.sin(Math.PI * t)), size * (0.8 + 0.35 * Math.sin(Math.PI * t)), fade, t * 6);
      for (let i = 1; i <= 4; i++) {
        const tt = clamp(t - i * 0.055, 0, 1);
        const trail = [lerp(from.x, to.x, tt), lerp(from.y, to.y, tt), lerp(from.z, to.z, tt) + 1.05 + Math.sin(Math.PI * tt) * 0.18];
        pushQuad(trail, size * (0.62 - i * 0.1), size * (0.62 - i * 0.1), fade * (0.5 - i * 0.09), -t * 4);
      }
      const impact = clamp((t - 0.82) / 0.18, 0, 1);
      for (let i = 0; i < 6; i++) {
        const a = plan.angles[i];
        const r = impact * (0.5 + plan.speeds[i] * 1.1);
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.25 + Math.sin(a * 2) * 0.35],
          size * 0.7 * (1 - impact * 0.6), size * 0.7 * (1 - impact * 0.6), (1 - impact) * fade);
      }
      break;
    }
    case 'meteor': {
      const fall = t * t;
      const head = [lerp(to.x - 2.2, to.x, fall), lerp(to.y + 2.2, to.y, fall), lerp(to.z + 9, to.z, fall)];
      pushQuad(head, size * 1.1, size * 1.1, fade, t * 8);
      for (let i = 1; i <= 4; i++) {
        const tt = clamp(fall - i * 0.05, 0, 1);
        pushQuad([lerp(to.x - 2.2, to.x, tt), lerp(to.y + 2.2, to.y, tt), lerp(to.z + 9, to.z, tt)],
          size * (0.8 - i * 0.12), size * (0.8 - i * 0.12), fade * (0.55 - i * 0.1), -t * 5);
      }
      const impact = clamp((t - 0.72) / 0.28, 0, 1);
      for (let i = 0; i < 6; i++) {
        const a = plan.angles[i];
        const r = impact * (0.4 + plan.speeds[i] * 1.6);
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.2 + impact * 0.7],
          size * (1.0 - impact * 0.5), size * (1.0 - impact * 0.5), (1 - impact) * (1 - impact));
      }
      break;
    }
    case 'column': {
      for (let i = 0; i < 6; i++) {
        const phase = (i / 6 + t) % 1;
        const a = plan.angles[i];
        const r = 0.18 + (i % 2) * 0.16;
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.15 + phase * 2.4 * size],
          size * (0.7 + phase * 0.5), size * (0.9 + phase * 0.7), (1 - phase) * fade, phase * 4);
      }
      break;
    }
    case 'ring': {
      out.push(disc(camera, [to.x, to.y, to.z + 0.08], (0.35 + 3.4 * t) * size * 0.7, (1 - t) * fade, t * 3));
      for (let i = 0; i < 8; i++) {
        const a = plan.angles[i] + t * 1.4;
        const r = (0.5 + 3.0 * t) * size * 0.7;
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.25 + Math.sin(Math.PI * t) * 0.8],
          size * 0.45 * (1 - t * 0.5), size * 0.45 * (1 - t * 0.5), (1 - t) * fade);
      }
      break;
    }
    case 'burst': {
      for (let i = 0; i < 10; i++) {
        const a = plan.angles[i] + i * 0.15;
        const r = (0.35 + 2.6 * t) * plan.speeds[i];
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.45 + (plan.speeds[i] - 0.9) * 1.4 + t * 0.9],
          size * (0.85 - t * 0.45), size * (0.85 - t * 0.45), (1 - t) * fade, t * 5);
      }
      break;
    }
    case 'bolt': {
      const top = [to.x, to.y, to.z + 8];
      for (let i = 0; i < 5; i++) {
        const a0 = i / 5, a1 = (i + 1) / 5;
        const jitter = (k) => (plan.speeds[k] - 0.9) * 0.55 * Math.sin(Math.PI * (a0 + (a1 - a0) * k));
        const p0 = [lerp(top[0], to.x, a0) + jitter(0), lerp(top[1], to.y, a0) + jitter(3), lerp(top[2], to.z, a0)];
        const p1 = [lerp(top[0], to.x, a1) + jitter(1), lerp(top[1], to.y, a1) + jitter(4), lerp(top[2], to.z, a1)];
        out.push(ribbon(camera, p0, p1, size * 0.55, fade * (0.65 + 0.35 * Math.sin(t * 40 + i)), undefined));
      }
      out.push(disc(camera, [to.x, to.y, to.z + 0.05], size * 0.9 * (0.6 + t), (1 - t) * fade, t * 2));
      break;
    }
    case 'arc': {
      const reach = clamp(t * 1.8, 0, 1);
      const dx = to.x - from.x, dy = to.y - from.y;
      const length = Math.hypot(dx, dy) || 1;
      const nx = -dy / length, ny = dx / length;
      for (let i = 0; i < 4; i++) {
        const spread = (i - 1.5) * 0.42;
        const cx = lerp(from.x, to.x, reach * 0.85) + nx * spread;
        const cy = lerp(from.y, to.y, reach * 0.85) + ny * spread;
        pushQuad([cx, cy, from.z + 0.55 + Math.sin(Math.PI * t) * 0.35],
          size * 1.5, size * 0.7, (1 - t * 0.7) * fade, Math.atan2(dy, dx) + spread);
      }
      break;
    }
    case 'aura': {
      out.push(disc(camera, [to.x, to.y, to.z + 0.06], size * (0.8 + 0.25 * Math.sin(t * 12)), (1 - t) * 0.9, t * 2));
      for (let i = 0; i < 4; i++) {
        const a = i * TAU / 4 + t * TAU * 1.2;
        pushQuad([to.x + Math.cos(a) * 0.72, to.y + Math.sin(a) * 0.72, to.z + 0.55 + Math.sin(a * 2 + t * 6) * 0.35],
          size * 0.55, size * 0.55, (1 - t) * fade, a);
      }
      break;
    }
    case 'beam': {
      out.push(ribbon(camera, [from.x, from.y, from.z + 0.55], [to.x, to.y, to.z + 0.55], size * 0.75, fade, undefined));
      for (let i = 0; i < 8; i++) {
        const f = i / 7;
        pushQuad([lerp(from.x, to.x, f), lerp(from.y, to.y, f), lerp(from.z, to.z, f) + 0.55 + Math.sin(f * 12 + t * 18) * 0.22],
          size * 0.42, size * 0.42, fade * (0.5 + 0.5 * Math.sin(t * 30 + i)), t * 4);
      }
      break;
    }
    case 'circle': {
      out.push(disc(camera, [to.x, to.y, to.z + 0.05], size * (0.7 + 0.5 * t), (1 - t) * fade, -t * 2));
      for (let i = 0; i < 8; i++) {
        const a = i * TAU / 8 + t * 2;
        const r = size * (0.5 + 0.25 * Math.sin(t * 8 + i));
        pushQuad([to.x + Math.cos(a) * r, to.y + Math.sin(a) * r, to.z + 0.2 + t * 1.8],
          size * 0.35, size * 0.35, (1 - t) * fade, a);
      }
      break;
    }
    case 'chain': {
      const targets = [
        to,
        {x: to.x + Math.cos(plan.angles[0]) * 1.2, y: to.y + Math.sin(plan.angles[0]) * 1.2, z: to.z},
        {x: to.x + Math.cos(plan.angles[1]) * 1.6, y: to.y + Math.sin(plan.angles[1]) * 1.6, z: to.z + 0.2}
      ];
      const points = [from, to, targets[1], targets[2]];
      for (let i = 0; i < 3; i++) {
        const a = points[i], b = points[i + 1];
        const mid = [lerp(a.x, b.x, 0.5), lerp(a.y, b.y, 0.5), lerp(a.z, b.z, 0.5) + 0.6 * Math.sin(Math.PI * t)];
        out.push(ribbon(camera, [a.x, a.y, a.z + 0.55], mid, size * 0.4, fade, undefined));
        out.push(ribbon(camera, mid, [b.x, b.y, b.z + 0.55], size * 0.4, fade, undefined));
      }
      break;
    }
    case 'shield': {
      for (let i = 0; i < 3; i++) {
        const height = 0.35 + i * 0.45;
        const radius = (0.95 - i * 0.22) * size;
        out.push(disc(camera, [to.x, to.y, to.z + height], radius, (1 - t) * fade, t * (i % 2 ? -3 : 3) + i));
      }
      break;
    }
    default:
      throw new Error('Unknown skill effect mode: ' + spec.mode);
  }
  return out;
}
