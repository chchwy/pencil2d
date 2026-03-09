# Crash Reporter Setup — Pencil2D + GlitchTip (sentry-native)

## What Was Implemented

Windows x64 crash reporting using the [sentry-native](https://github.com/getsentry/sentry-native)
SDK (Sentry-protocol compatible, works with [GlitchTip](https://glitchtip.com)).

### Files Added / Changed

| File | What Changed |
|------|-------------|
| `app/src/crashhandler.h` | New — `CrashHandler` class declaration |
| `app/src/crashhandler.cpp` | New — wraps `sentry_init()` / `sentry_close()` |
| `util/sentry.pri` | New — qmake include; activates when `SENTRY_DSN` is passed |
| `app/app.pro` | Includes `sentry.pri`; adds `crashhandler.*` to build |
| `app/src/main.cpp` | Calls `CrashHandler::init()` at startup, `close()` at exit |
| `.github/actions/install-dependencies/action.yml` | New `sentry_dsn` input |
| `.github/actions/install-dependencies/install-dependencies.sh` | Builds sentry-native 0.7.17 for Windows x64 when DSN is set |
| `.github/workflows/ci.yml` | Passes `secrets.SENTRY_DSN` to install step and to qmake |
| `.github/actions/create-package/create-package.sh` | Harvests `sentry.dll` + `crashpad_handler.exe` into WiX installer |

### How It Works

- Crash reporting is **off by default** — no code runs, no network calls, no overhead.
- It activates only when the `SENTRY_DSN` build variable is set.
- On Windows x64 (MSVC), the crashpad backend captures crashes out-of-process and
  queues them for upload to GlitchTip on the next run.
- Three environments are reported automatically:
  - `production` — release builds (tagged versions)
  - `nightly` — nightly builds (`99.0.0.x`)
  - `development` — local dev builds (`0.0.0.0`)

---

## Next Steps

### 1. Set Up a GlitchTip Project

- [ ] Create an account at [glitchtip.com](https://glitchtip.com) (or self-host).
- [ ] Create a new **project** (type: *Native*).
- [ ] Copy the **DSN** — it looks like `https://<key>@app.glitchtip.com/<project-id>`.

### 2. Add the GitHub Secret

- [ ] Go to the pencil2d GitHub repo → **Settings → Secrets and variables → Actions**.
- [ ] Add a new secret named exactly `SENTRY_DSN` with the DSN value from step 1.
- [ ] The CI will automatically enable crash reporting in Windows x64 builds once the
      secret exists. No code changes needed.

### 3. Build and Verify the sentry-native SDK Version

The CI currently pins sentry-native **0.7.17**. Before merging, verify this tag exists:

```
https://github.com/getsentry/sentry-native/releases/tag/0.7.17
```

If it does not exist, update the tag in
`.github/actions/install-dependencies/install-dependencies.sh`:

```bash
git clone --depth=1 --branch 0.7.17 ...   # ← change version here
```

Pick the latest stable release from the [releases page](https://github.com/getsentry/sentry-native/releases).

### 4. Test a Local Build (Windows x64, MSVC)

Build sentry-native once manually:

```bat
git clone --depth=1 --branch 0.7.17 --recurse-submodules ^
    https://github.com/getsentry/sentry-native.git third_party\sentry-native

cmake -S third_party\sentry-native -B third_party\sentry-native\build ^
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
    -DSENTRY_BACKEND=crashpad ^
    -DCMAKE_INSTALL_PREFIX=third_party\sentry-native\install

cmake --build third_party\sentry-native\build --config RelWithDebInfo --parallel 4
cmake --install third_party\sentry-native\build --config RelWithDebInfo
```

Then configure Pencil2D with the DSN:

```bat
qmake SENTRY_DSN="https://<key>@app.glitchtip.com/<id>"
nmake
```

Run the app and confirm:
- [ ] `crashpad_handler.exe` appears in the build output directory.
- [ ] `sentry.dll` appears in the build output directory.
- [ ] The app launches and closes without errors.
- [ ] A crash database folder appears under `%LOCALAPPDATA%\Pencil2D\Pencil2D\crashes\`.

### 5. Trigger a Test Crash

To confirm end-to-end reporting works, temporarily add a forced crash and check
GlitchTip receives it.

In `app/src/main.cpp`, right after `CrashHandler::init(...)`:

```cpp
// TEMPORARY — remove after confirming GlitchTip receives the report
sentry_capture_event(sentry_value_new_message_event(
    SENTRY_LEVEL_FATAL, "test", "Test crash from Pencil2D"));
```

Or trigger a real crash:

```cpp
int* p = nullptr;
*p = 0;  // TEMPORARY — force crash for testing
```

After confirming a report appears in GlitchTip, remove the test code.

### 6. Upload Debug Symbols (PDB files)

Without symbols, crash stack traces in GlitchTip will show raw addresses.
To get readable stack traces, upload PDB files after each CI build.

- [ ] Install [sentry-cli](https://docs.sentry.io/product/cli/installation/).
- [ ] Add a CI step after the build:

```yaml
- name: Upload debug symbols
  if: runner.os == 'Windows' && matrix.arch == 'win64_msvc2019_64' && env.SENTRY_DSN != ''
  env:
    SENTRY_DSN: ${{ secrets.SENTRY_DSN }}
    SENTRY_AUTH_TOKEN: ${{ secrets.SENTRY_AUTH_TOKEN }}
    SENTRY_ORG: your-org-slug
    SENTRY_PROJECT: pencil2d
  run: |
    sentry-cli debug-files upload --include-sources build/
```

- [ ] Add `SENTRY_AUTH_TOKEN` as a GitHub secret (generate in GlitchTip under
      **Profile → Auth Tokens**).
- [ ] Replace `your-org-slug` with your actual GlitchTip organization slug.

> **Note:** GlitchTip's support for `sentry-cli debug-files upload` is in progress.
> Check [GlitchTip issue tracker](https://gitlab.com/glitchtip/glitchtip-backend/-/issues)
> for the current status of symbol server support.

### 7. macOS and Linux (Deferred)

The current implementation is Windows x64 (MSVC) only. To add other platforms:

- **macOS**: sentry-native supports crashpad on macOS. Add a `macx` block in
  `util/sentry.pri` and a macOS build step for sentry-native in CI.
- **Linux**: sentry-native supports breakpad on Linux. Add a `unix:!macx` block
  in `util/sentry.pri` and a Linux build step in CI.

The `CrashHandler` class itself requires no changes — only the qmake and CI
plumbing needs to be extended per platform.

### 8. Consider User Consent (Optional)

Some jurisdictions and app stores require users to opt in to crash reporting.
If needed, add a preferences toggle in `app/src/generalpage.*` and call:

```cpp
sentry_options_set_require_user_consent(options, 1); // in CrashHandler::init
// then later, when user opts in:
sentry_user_consent_give();
// or if user opts out:
sentry_user_consent_revoke();
```

---

## Quick Reference

| What | Where |
|------|-------|
| Crash handler wrapper | `app/src/crashhandler.h` / `.cpp` |
| qmake integration | `util/sentry.pri` |
| Enable crash reporting | Add `SENTRY_DSN` secret to GitHub repo |
| sentry-native version | `0.7.17` (in `install-dependencies.sh`) |
| Crash DB on Windows | `%LOCALAPPDATA%\Pencil2D\Pencil2D\crashes\` |
| GlitchTip docs | https://glitchtip.com/documentation |
| sentry-native releases | https://github.com/getsentry/sentry-native/releases |
