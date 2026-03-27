---
name: update-translations
description: This skill should be used when the user asks to "update translations", "pull translations", "sync translations from Transifex", "run lupdate", or wants to update the Pencil2D translation files.
version: 1.0.0
---

# Update Translations Skill

Keeps Pencil2D's translation files up to date by pulling from Transifex and refreshing the source strings via lupdate.

## Steps

Execute the following steps in order. Stop and report to the user if any step fails.

### 1. Ensure Qt is in PATH

`lupdate` must be available. Check first:

```bash
which lupdate
```

If not found, ask the user to run `qt6` or `qt5` in their shell to add Qt to PATH, then try again. The project supports Qt 5 and Qt 6.

### 2. Pull translations from Transifex

Run from the project root:

```bash
python3 util/transifex_pull.py
```

The script reads `TRANSIFEX_TOKEN` from the environment. If it fails with an auth error, remind the user to `source ~/.zshrc` or set the token manually.

### 3. Update the translation source file via lupdate

Run from the project root:

```bash
lupdate pencil2d.pro
```

This scans all C++/UI source files and updates `translations/pencil.ts` with new/removed strings. Review the output — lupdate will report how many strings were found, added, or removed.

### 4. Stage the translation files

Stage all changes in the translations directory:

```bash
git add translations/
```

This captures both modified existing `.ts` files and any newly added language files.

### 5. Create the git commit

Use this commit message format:

```
Update translations

- Pulled latest translations from Transifex
- Updated source strings in pencil.ts via lupdate
```

If new language files were added (untracked `.ts` files), list them in the commit body, e.g.:

```
Update translations

- Pulled latest translations from Transifex
- Updated source strings in pencil.ts via lupdate
- Added new languages: Filipino (fil), Hindi (hi_IN), Romanian (ro)
```

Do NOT push. Tell the user the commit is ready for their review.

## Notes

- `translations/pencil.ts` is the **source** file (no human translations — updated by lupdate from C++ source)
- `translations/pencil_XX.ts` files are the **language** files (translated strings — pulled from Transifex)
- Only commit after both steps (Transifex pull + lupdate) have completed successfully
