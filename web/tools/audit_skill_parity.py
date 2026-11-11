#!/usr/bin/env python3
"""Read-only parity audit: PC client Skill_eng.bmd, server SkillList, web catalog/effects."""
import argparse, hashlib, json, re, struct, shlex
from pathlib import Path

MASTER_ACTIVE = {344, 425, 426, 427, 430, 432, 461, 495, 521, 565}
NO_VISUAL = {212, 344, 495, 521, 565}
NO_CLIENT_VISUAL = {
    212: "AT_SKILL_CURSED_TEMPLE_TELEPORT chỉ SetAction(PLAYER_HIT_REMOVAL), không có Create*",
    344: "Skill_Blood_Storm chỉ có enum + xử lý server, không có case/Create* trong client",
    495: "Skill_Earth_Prison chỉ có buff icon EFFECT_EARTH_PRISON, không có cast effect client",
    521: "Skill_Iron_Defense chỉ có buff icon EFFECT_IRON_DEFENSE, không có cast effect client",
    565: "Skill_Blood_Howling chỉ có buff icon EFFECT_BLOOD_HOWLING, không có cast effect client",
}
FAMILY_LINE = re.compile(r"^\s*(\d+):\s*spec\('([^']+)',\s*'([^']+)'(?:,\s*'([^']*)')?")


def client_records(path):
    raw = path.read_bytes(); size = 88
    if len(raw) < 650 * size:
        raise ValueError(f"Skill_eng.bmd too small: {len(raw)}")
    buf = bytearray(raw[:650 * size])
    for rec in range(650):
        for i in range(size):
            buf[rec * size + i] ^= (0xfc, 0xcf, 0xab)[i % 3]
    out = {}
    for i in range(650):
        b = buf[i * size:(i + 1) * size]
        name = b[:32].split(b"\0")[0].decode("latin1", "replace").strip()
        if not name:
            continue
        out[i] = {
            "id": i, "name": name,
            "level": struct.unpack_from("<H", b, 32)[0],
            "damage": struct.unpack_from("<H", b, 34)[0],
            "mana": struct.unpack_from("<H", b, 36)[0],
            "ag": struct.unpack_from("<H", b, 38)[0],
            "range": struct.unpack_from("<I", b, 40)[0],
            "delay": struct.unpack_from("<i", b, 44)[0],
            "energy": struct.unpack_from("<i", b, 48)[0],
            "classes": list(b[64:71]),
        }
    return out


def server_rows(path):
    rows = []
    for line in path.read_text(encoding="latin1").splitlines():
        line = line.split("//")[0].strip()
        if not line or line == "end":
            continue
        v = shlex.split(line)
        if len(v) != 22:
            continue
        rows.append({
            "id": int(v[0]), "name": v[1], "mana": int(v[3]), "ag": int(v[4]),
            "range": int(v[5]), "radius": int(v[6]), "delay": int(v[7]),
            "level": int(v[10]), "energy": int(v[11]),
            "classes": list(map(int, v[15:22])),
        })
    return rows


def effect_specs(path):
    specs = {}
    for line in path.read_text().splitlines():
        m = FAMILY_LINE.match(line)
        if not m:
            continue
        sid, family, mode, source = int(m.group(1)), m.group(2), m.group(3), m.group(4)
        specs[sid] = {"family": family, "mode": mode, "source": source or None,
                      "exact": "{exact: true}" in line}
    return specs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", type=Path, required=True)
    ap.add_argument("--client", type=Path, required=True)
    ap.add_argument("--server", type=Path, required=True)
    ap.add_argument("--web", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    a = ap.parse_args()

    client = client_records(a.client / "Local/Eng/Skill_eng.bmd")
    server = server_rows(a.server / "Skill/SkillList.txt")
    catalog_text = (a.web / "client/src/ui/skill-catalog.js").read_text()
    catalog = json.loads(re.search(r"export const skillCatalog=(\{.*\});", catalog_text, re.S).group(1))
    rows = catalog["skills"]
    specs = effect_specs(a.web / "client/src/world/skill-effect-catalog.js")

    cm = {r["id"]: r for r in client.values()}
    sm = {r["id"]: r for r in server}
    web = {r["id"]: r for r in rows}
    active = [r for r in rows if r["classes"] and r["classes"].count(0) < 7 and
              ((r["id"] < 300 and (r["mana"] > 0 or r["ag"] > 0)) or r["id"] in MASTER_ACTIVE)]
    covered = [r for r in active if r["id"] in specs or r["id"] in NO_VISUAL]
    exact = [r["id"] for r in active if specs.get(r["id"], {}).get("exact")]
    sourced = [r["id"] for r in active if specs.get(r["id"], {}).get("source") and not specs[r["id"]]["exact"]]
    derived = [r["id"] for r in active if r["id"] in specs and not specs[r["id"]]["exact"] and not specs[r["id"]]["source"]]
    no_visual = [r["id"] for r in active if r["id"] in NO_VISUAL]

    def diff(field):
        return [(i, cm[i].get(field), sm[i].get(field)) for i in sorted(set(cm) & set(sm))
                if cm[i].get(field) != sm[i].get(field)]

    name_diff = [(i, cm[i]["name"], sm[i]["name"]) for i in sorted(set(cm) & set(sm)) if cm[i]["name"] != sm[i]["name"]]
    class_diff = [(i, cm[i]["classes"], sm[i]["classes"]) for i in sorted(set(cm) & set(sm)) if cm[i]["classes"] != sm[i]["classes"]]
    range_diff = [(i, cm[i]["range"], sm[i]["range"], sm[i]["radius"]) for i in sorted(set(cm) & set(sm)) if cm[i]["range"] != sm[i]["range"]]

    zzz = (a.source / "ZzzCharacter.cpp").read_text(errors="replace")
    cases = re.findall(r"case\s+((?:AT_SKILL_|Skill_)[A-Za-z0-9_]+)\s*:", zzz)
    effect_calls = len(re.findall(r"\bCreate(?:Effect|Joint|Particle|Sprite)\s*\(", zzz))

    lines = []
    lines.append("# Audit parity Skill / Effect — 2026-11-04\n")
    lines.append("Sinh bằng `web/tools/audit_skill_parity.py`. Chỉ đọc source/data, không sửa gameplay.\n")
    lines.append("## Nguồn\n")
    lines.append(f"- PC client: `Skill_eng.bmd` — **{len(client)}** skill có tên (650 record).")
    lines.append(f"- Server: `SkillList.txt` — **{len(server)}** skill.")
    lines.append(f"- Web catalog: **{len(rows)}** skill.")
    lines.append(f"- PC effect dispatch: **{len(set(cases))}** tên `case` duy nhất, **{effect_calls}** call `Create*` trong `ZzzCharacter.cpp`.\n")
    lines.append("## Metadata\n")
    lines.append(f"- Client-only: **{len(set(cm)-set(sm))}** — `{sorted(set(cm)-set(sm))}`")
    lines.append(f"- Server-only: **{len(set(sm)-set(cm))}** — `{sorted(set(sm)-set(cm))}`")
    lines.append(f"- Tên khác: **{len(name_diff)}** — `{name_diff}`")
    lines.append(f"- Class khác: **{len(class_diff)}** — `{class_diff}`")
    lines.append(f"- Range khác: **{len(range_diff)}** — `{range_diff[:40]}`")
    lines.append("")
    lines.append("Các field `level`/`energy` của client và `ReqLevel`/`ReqEnergy` của server khác ngữ nghĩa; không dùng để kết luận lỗi nếu chưa xác minh.\n")
    lines.append("## Effect coverage\n")
    lines.append(f"- Active: **{len(active)}**; có spec: **{len(covered)}**; missing: **{len(active)-len(covered)}**.")
    lines.append(f"- Exact: **{len(exact)}** `{exact}`")
    lines.append(f"- Source token: **{len(sourced)}**")
    lines.append(f"- Derived (không có token nguồn riêng): **{len(derived)}** — `{derived}`")
    lines.append(f"- No visual (source xác nhận không có cast effect): **{len(no_visual)}** — `{no_visual}`")
    lines.append("")
    lines.append("Derived chỉ là họ hình ảnh gần đúng; chưa chứng minh hình học/vật liệu/timing/âm thanh.\n")
    notes = [f"- `{sid}`: {NO_CLIENT_VISUAL[sid]}" for sid in no_visual if sid in NO_CLIENT_VISUAL]
    if notes:
        lines.append("### No-visual trong PC client")
        lines.extend(notes)
        lines.append("")
    lines.append("## Skill active chưa có spec\n")
    missing = [r for r in active if r["id"] not in specs and r["id"] not in NO_VISUAL]
    lines.append(f"`{[r['id'] for r in missing]}`\n" if missing else "Không có.\n")
    lines.append("## Việc cần làm\n")
    lines.append("1. Chốt field metadata nào là chuẩn (PC client hay server custom).")
    lines.append(f"2. Port effect thật cho {len(derived)} derived / các nhánh `Create*` chưa map.")
    lines.append("3. Diff damage/range/cooldown/buff server ↔ PC; kiểm tra master skill tree và buff UI.")
    a.out.parent.mkdir(parents=True, exist_ok=True)
    a.out.write_text("\n".join(lines) + "\n")
    print("wrote", a.out)
    print("active", len(active), "covered", len(covered), "exact", len(exact), "source", len(sourced), "derived", len(derived), "noVisual", len(no_visual))

if __name__ == "__main__":
    main()
