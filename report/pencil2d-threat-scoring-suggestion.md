# Threat Scoring Suggestion — Pencil2D Desktop Application

> **⚠️ This is an AI-generated scoring suggestion — not the official assessment.**
> The formal threat scoring in the threat model report (§6) is to be filled by the security engineers. Use this file as a reference and starting point.

**Companion to**: `pencil2d-threat-model.md`

---

## Scoring Method

- **Threats**: light **Likelihood × Impact** matrix, prioritise rather than audit.
- **Vulnerabilities**: CVSS v4.0 for severity, distinct from the L×I used here.
- **Questions**: worst-case estimate for the unknowns; lower after investigation.
- **Attacker-model anchor (Assumption #1)**: remote attacker, no local access, user opens/imports a crafted file. Baseline Likelihood for "remote, user must open a file" starts around **Likely (3)** for the file-open path (opening shared project files is the app's normal use), and lower for chains needing extra conditions (multi-instance, shared `/tmp`, unattended CLI, macOS-only timing). Deltas are noted per row.

## Risk Matrix

| | Very unlikely | Not likely | Likely | Very likely | Extremely likely |
|---|---|---|---|---|---|
| **Extreme** | 5 | 10 | 15 | 20 | 25 |
| **Severe** | 4 | 8 | 12 | 16 | 20 |
| **Substantial** | 3 | 6 | 9 | 12 | 15 |
| **Moderate** | 2 | 4 | 6 | 8 | 10 |
| **Slight** | 1 | 2 | 3 | 4 | 5 |

| Score Range | Level | Color |
|-------------|-------|-------|
| 16–25 | Very high risk | Red |
| 11–15 | High risk | Orange |
| 6–10 | Moderate risk | Yellow |
| 1–5 | Low risk | Green |

## Suggested Scoring Table

| **Threat ID / Question ID** | **Score** | **Tracking** | **Notes** |
| --------------------------- | --------- | ------------ | --------- |
| T.1 | 15 (L3 × Extreme5) | TBD | Zip-slip traversal verified with a harness; write-outside is certain once a file is opened. Impact Extreme (A2/A3, code execution). Likelihood Likely, not higher, because the code-execution half (Startup/binary planting) is pending dynamic confirmation (Q.1) and depends on default install/TEMP layout. |
| T.2 | 12 (L3 × Severe4) | TBD | Amplifies T.1. Requires the write-primitive plus a media operation (or next launch). Impact Severe (persistent execution). Score with T.1; treat as one chain. |
| T.3 | 12 (L3 × Severe4) | TBD | Outdated FFmpeg 4.1.1 / Qt 5.15.2 parse attacker media. Likelihood Likely (importing media is normal). Impact Severe (memory-corruption code exec) but depends on a live CVE in the shipped build (Q.4). |
| T.4 | 6 (L2 × Substantial3) | TBD | Legacy `.pcl` symlink disclosure. Likelihood Not likely — needs a delivery channel that preserves symlinks and the user to re-share the saved project. Impact Substantial (A8 credential/file leak). |
| T.5 | 8 (L2 × Severe4) | TBD | UNC entry → NTLM hash leak, Windows only. Likelihood Not likely (needs outbound SMB allowed). Impact Severe (credential theft enables lateral movement). |
| T.6 | 12 (L4 × Substantial3) | TBD | Crash/hang/UB on open. Likelihood Very likely — trivial to craft, fires on open. Impact Substantial (availability; UB possibly worse pending Q.2). |
| T.7 | 12 (L4 × Substantial3) | TBD | Persistent DoS via recovery re-offer. Likelihood Very likely once T.6 fires. Impact Substantial (user cannot start work until temp cleared). |
| T.8 | 9 (L3 × Substantial3) | TBD | Zip bomb / unbounded media. Likelihood Likely. Impact Substantial (temp/mem exhaustion, recoverable). |
| T.9 | 6 (L2 × Substantial3) | TBD | Recovery spoofing. Likelihood Not likely (needs a prior planted dir and the user to accept "restore"). Impact Substantial (foreign content saved as own work). |
| T.10 | 6 (L2 × Substantial3) | TBD | Shared-`/tmp` local tampering — secondary attacker, off the primary model. Likelihood Not likely (needs a local user). Impact Substantial (data loss/disclosure). |
| T.11 | 4 (L2 × Moderate2) | TBD | Rich-text injection. Likelihood Not likely to matter (spoofing only unless `<img>` file-loading works — Q.10). Impact Moderate. |
| T.12 | 4 (L2 × Moderate2) | TBD | Clipboard auto-decode. Likelihood Not likely (needs a bitmap layer active + clipboard writer). Impact Moderate (decoder crash). |
| T.13 | 8 (L2 × Severe4) | TBD | Supply-chain via unsigned/unpinned distribution. Likelihood Not likely (needs a pipeline/host compromise) but Impact Extreme in principle; scored Severe pending Q.5 on out-of-band signing. Whole-user-base blast radius. |
| T.14 | 6 (L2 × Substantial3) | TBD | Unattended CLI exhaustion. Likelihood Not likely (needs CLI rendering of untrusted projects — Q). Impact Substantial. |
| T.15 | 6 (L2 × Substantial3) | TBD | Re-entrancy/autosave corruption. Likelihood Not likely (timing/macOS-specific, or autosave enabled). Impact Substantial (data loss). |
| Q.1 | 20 (L4 × Extreme5) | TBD | Worst case: the zip-slip → code-exec chain works out of the box on shipped Windows. If confirmed, T.1/T.2 rise toward this. Lower after dynamic test. |
| Q.2 | 20 (L4 × Extreme5) | TBD | Worst case: `Q_UNREACHABLE` switches drop bounds checks → controllable wild jump (exploitable, not just a crash). Lower to ~12 if release codegen only crashes. |
| Q.3 | 8 (L2 × Severe4) | TBD | Worst case: recovery adopts/destroys foreign or live data broadly. Design change scoped by answer. |
| Q.4 | 15 (L3 × Extreme5) | TBD | Worst case: a deployed image plugin or the bundled FFmpeg has a live RCE-class CVE reachable on import. Lower once versions/CVEs are enumerated. |
| Q.5 | 8 (L2 × Severe4) | TBD | Worst case: no out-of-band signing and writable `pencil2d-deps` assets → supply-chain injection. Lower if signing exists. |
| Q.6 | 6 (L3 × Substantial3) | TBD | Worst case: recovery always rebuilds from filenames on Windows too, dropping layers on every recovery. |
| Q.7 | 4 (L2 × Moderate2) | TBD | Worst case: no sandboxed Linux package, so F19 applies to all Linux users. Lower if Flatpak/Snap dominates. |
| Q.8 | 8 (L2 × Severe4) | TBD | Worst case: FileOpen during a modal op reliably triggers use-after-free on macOS. |
| Q.9 | 6 (L2 × Substantial3) | TBD | Worst case: default preset auto-loads a crafted project through the full loader on every start (persistence). |
| Q.10 | 8 (L2 × Severe4) | TBD | Worst case: ErrorDialog rich text loads `file://host/share` resources → SMB/NTLM leak from a crash message. |

> Scores are worst-case anchors. Reassess T.1/T.2 and Q.1/Q.2 together after the dynamic Windows test, and T.3/Q.4 after the deployed-decoder inventory.
