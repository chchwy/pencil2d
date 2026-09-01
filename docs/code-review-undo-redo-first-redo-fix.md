# Code Review: `undo-redo-first-redo-fix`

**Branch:** `undo-redo-first-redo-fix`
**Commit:** `08913d24b` — *Move isFirstRedo to undo manager*
**Base:** `origin/master` (`05171450f`)
**Files:** 4 files, +18 / −9
**Verdict:** Do not approve as-is.

---

## Summary / 摘要

### English

The change moves `isFirstRedo` from a per-command boolean onto `UndoRedoManager`, set around `QUndoStack::push()`. That is the right invariant: `push()` always calls `redo()`, and these commands already applied the edit in their constructors, so the automatic redo must be a no-op.

The flag moved. The **decision** did not. Every derived `redo()` still special-cases first-redo itself. That is the same parent/child coupling the commit message is trying to escape.

### 中文

這次改動把 `isFirstRedo` 從每個 command 上的布林值，搬到 `UndoRedoManager`，在 `QUndoStack::push()` 期間設為 true。語意是對的：`push()` 一定會呼叫 `redo()`，而這些 command 在建構子裡已經套用過編輯結果，因此這次自動 `redo()` 必須是空操作。

旗標搬家了，**判斷邏輯沒有**。每個衍生類的 `redo()` 仍然自己處理 first-redo。這正是 commit message 想擺脫的 parent/child 耦合。

---

## What changed / 改了什麼

`QUndoStack::push(cmd)` always calls `cmd->redo()`. Pencil2D’s new undo commands snapshot the already-applied state in the constructor, so they skip that first automatic redo.

| Before / 改前 | After / 改後 |
|---|---|
| Each `UndoRedoCommand` owns `mIsFirstRedo = true`. First `redo()` skips work and sets the flag false. | Manager sets `mFirstUndoInProgress = true` around `mUndoStack.push()`, then false. Commands ask the manager. |

Commit rationale / commit 說明：

> Because it causes issues that each undo/redo command owns the isFirstRedo when parent is ignored and thus doesn't trigger redo on the child.
>
> 每個 undo/redo command 各自擁有 `isFirstRedo`。當 parent 被忽略（略過自動 redo）時，child 的 redo 不會被觸發。

The real failure mode is `replaceBitmap` / `replaceVector`: a `BitmapReplaceCommand` / `VectorReplaceCommand` parent with a `TransformCommand` child.

真正出問題的路徑是 `replaceBitmap` / `replaceVector`：parent 是點陣/向量取代 command，child 是 `TransformCommand`。

---

## Findings / 審查意見

Priority: structural first, then naming/API, then RAII, then tests.

優先順序：結構問題、命名與 API、RAII、測試。

---

### 1. Blocker — First-redo is still a per-command branch

**Files:** `core_lib/src/interface/undoredocommand.cpp`

#### English

This is the blocker.

The manager now owns “we are inside `push()`”. What did not change: every derived `redo()` still does this:

```cpp
UndoRedoCommand::redo(); // children
if (isFirstRedo()) { return; }
// actual work
```

Copied in all six commands: `KeyFrameRemoveCommand`, `KeyFrameAddCommand`, `MoveKeyFramesCommand`, `BitmapReplaceCommand`, `VectorReplaceCommand`, `TransformCommand`.

Virtual dispatch goes to the derived `redo()` first. The base class cannot intercept. So the duplicated `if` is not a style nit; it is the only thing keeping the invariant.

Consequences:

- A new command that forgets the check will double-apply on push.
- A parent that returns before calling the base still desyncs from its children. The global flag papers over that instead of making the flow obvious.
- During the dummy push-redo, children are still invoked (`UndoRedoCommand::redo()` runs *before* the check). With a manager-level flag that is wasted work, not required behavior.

The flag relocation without collapsing those branches is half a refactor: complexity moved, not removed.

**Remedy: Non-Virtual Interface (NVI).** Make `UndoRedoCommand::redo()` / `undo()` `final`. Derived classes implement `redoImpl()` / `undoImpl()`:

```cpp
void UndoRedoCommand::redo()
{
    // QUndoStack::push() always calls redo(); the edit is already applied.
    if (isFirstRedo())
        return;

    QUndoCommand::redo(); // children, only on a real redo
    redoImpl();
}

void UndoRedoCommand::undo()
{
    QUndoCommand::undo();
    undoImpl();
}
```

That deletes six identical branches, makes it impossible to forget the guard, and skips the whole tree during push so parent/child cannot desync.

Do this in this PR. Shipping the flag move alone leaves the design that already failed once.

#### 中文

這是阻擋合併的問題。

Manager 現在擁有「我們正在 `push()` 裡」這個事實。沒改到的是：每個衍生類的 `redo()` 仍然自己寫：

```cpp
UndoRedoCommand::redo(); // 先 redo children
if (isFirstRedo()) { return; }
// 真正的工作
```

六個 command 各貼一份：`KeyFrameRemoveCommand`、`KeyFrameAddCommand`、`MoveKeyFramesCommand`、`BitmapReplaceCommand`、`VectorReplaceCommand`、`TransformCommand`。

虛擬函式會先派到衍生類的 `redo()`，基底類攔截不到。所以這六個 `if` 不是風格問題，而是目前唯一在維持不變量的東西。

後果：

- 新 command 漏寫這個檢查，`push` 時會套用兩次。
- parent 若在呼叫 base `redo()` 之前就 return，child 仍然會不同步。全域旗標只是把這件事蓋過去，流程並沒有變清楚。
- dummy push-redo 期間仍然會呼叫 children（`UndoRedoCommand::redo()` 在檢查 **之前** 就執行了）。旗標已經在 manager 上，這段是多餘的工作，不是必要行為。

只搬家旗標、不收掉這些分支，是半套重構：複雜度換了位置，沒有刪掉。

**改法：Non-Virtual Interface（NVI）。** 讓 `UndoRedoCommand::redo()` / `undo()` 設成 `final`，衍生類實作 `redoImpl()` / `undoImpl()`（見上方英文範例）。

六個相同分支可以刪掉，不可能再漏寫 guard，`push` 期間整棵 command 樹都略過，parent/child 不會再不同步。

請在這個 PR 做完。只合旗標搬家，等於把已經失敗過一次的設計再送上去。

---

### 2. The manager API lies about the invariant / Manager API 名實不符

**Files:** `core_lib/src/managers/undoredomanager.h:141`, `:224`  
**Also:** `core_lib/src/interface/undoredocommand.h` (`isFirstRedo()`)

#### English

Three names for one fact, and they disagree:

| Symbol | What it says | What it actually is |
|---|---|---|
| `mFirstUndoInProgress` | an undo is running | `QUndoStack::push()` is running, which calls **redo** |
| `isFirstRedoInProgress()` | a redo is running | the same flag |
| `UndoRedoCommand::isFirstRedo()` | *this* command’s first redo | ignores `this`; asks the manager |

`isFirstRedo()` on a command instance now means “is the stack currently pushing,” not “has this command been redone.” Readers will get that wrong.

`isFirstRedoInProgress()` is also public on `UndoRedoManager`, sitting next to `hasUnsavedChanges()` with no comment. It is a push-call implementation detail. Feature code that goes through `editor()->undoRedo()` should not see it.

**Remedy:** Hide it (`friend class UndoRedoCommand`, or a private accessor). Name the member after the actual event, e.g. `mPushRedoInProgress`. If a command-side helper remains, name it for the stack operation (`isAutomaticPushRedo()`), not for per-command lifetime. NVI makes that helper unnecessary.

#### 中文

同一個事實用了三個名字，而且互相打架：

| 符號 | 字面上像是 | 實際上是 |
|---|---|---|
| `mFirstUndoInProgress` | 正在 undo | 正在 `QUndoStack::push()`，而 `push` 呼叫的是 **redo** |
| `isFirstRedoInProgress()` | 正在 redo | 同一個旗標 |
| `UndoRedoCommand::isFirstRedo()` | **這個** command 的第一次 redo | 不用 `this`，去問 manager |

Command 實例上的 `isFirstRedo()` 現在的意思是「stack 正在 push」，不是「這個 command 有沒有 redo 過」。之後讀程式的人會讀錯。

`isFirstRedoInProgress()` 還是 `UndoRedoManager` 的 public API，跟 `hasUnsavedChanges()` 放在一起，沒有註解。這是 `push` 實作細節。走 `editor()->undoRedo()` 的功能程式不該看到它。

**改法：** 藏起來（`friend class UndoRedoCommand`，或 private accessor）。成員名稱對準真正的事件，例如 `mPushRedoInProgress`。若 command 端還要 helper，名稱應對準 stack 操作（`isAutomaticPushRedo()`），不要再用「這個 command 的第一次 redo」。若做了 NVI，這個 helper 可以一併刪掉。

---

### 3. `pushCommand` is not RAII-safe / `pushCommand` 沒有用 RAII 管旗標

**File:** `core_lib/src/managers/undoredomanager.cpp:171-178`

```cpp
void UndoRedoManager::pushCommand(QUndoCommand* command)
{
    mFirstUndoInProgress = true;
    mUndoStack.push(command);
    mFirstUndoInProgress = false;

    emit didUpdateUndoStack();
}
```

#### English

This boolean must be true for exactly one stack frame. That is a guard object, not two assignments.

If `redo()` throws, the flag stays `true` and every later real redo becomes a no-op. Qt `redo()` is not `noexcept`.

A nested `pushCommand` during an in-flight `redo()` would also clear the outer flag when the inner `push` returns. Dummy `redo()` currently returns before emitting `frameModified`, so that path is probably dead today. A boolean still cannot express nested scope; a depth counter or a guard that restores the previous value can.

```cpp
void UndoRedoManager::pushCommand(QUndoCommand* command)
{
    const QScopedValueRollback firstRedo(mPushRedoInProgress, true);
    mUndoStack.push(command);
    emit didUpdateUndoStack();
}
```

`QScopedValueRollback` is already in Qt; no new helper type required.

#### 中文

這個布林值必須只在一個 stack frame 裡為 true。這是 guard 物件該做的事，不是兩行賦值。

若 `redo()` 丟出例外，旗標會一直停在 `true`，之後每一次真正的 redo 都會變成空操作。Qt 的 `redo()` 不是 `noexcept`。

若 `redo()` 進行中又進了一次 `pushCommand`，內層 `push` 返回時會把外層旗標清掉。目前 dummy `redo()` 在 emit `frameModified` 之前就 return，這條路徑今天大概走不到。但布林值本來就表達不了巢狀範圍；深度計數，或還原「進入前舊值」的 guard 才可以。

用 `QScopedValueRollback` 即可（Qt 現成的，不必自寫型別）。見上方英文範例。

---

### 4. No test for the parent/child case this commit exists to fix / 沒有覆蓋這次要修的 parent/child 案例

**Related code:** `UndoRedoManager::replaceBitmap` / `replaceVector` (`undoredomanager.cpp:221-261`)  
**Tests:** `tests/src` has no undo/redo coverage.

#### English

`replaceBitmap` / `replaceVector` are the only parent/child commands:

```cpp
BitmapReplaceCommand* element = new BitmapReplaceCommand(...);
new TransformCommand(..., editor(), element);
pushCommand(element);
```

That is the failure mode in the commit message. There is no test.

A test that enables the new undo system, records a bitmap (or vector) modify with a selection, undoes, redoes, and asserts **both** the image **and** the transform came back would lock the actual bug. Without it, the next command that overrides `redo()` can reintroduce the skip.

#### 中文

`replaceBitmap` / `replaceVector` 是目前唯一的 parent/child command（見上方英文程式）。這就是 commit message 裡的失敗模式。`tests/src` 沒有任何 undo/redo 測試。

需要一則測試：打開新 undo 系統，記錄一次帶選取範圍的點陣（或向量）修改，undo 再 redo，斷言 **圖像與 transform 都回來了**。沒有這則測試，下一個覆寫 `redo()` 的 command 隨時可以把「略過 child」的 bug 帶回來。

---

## What is fine / 沒問題的部分

### English

- Putting “automatic push redo” on the manager instead of on each `QUndoCommand` is the right model.
- `UndoRedoCommand::isFirstRedo()` as a forwarding helper is a reasonable isolation layer *until* NVI makes it go away.
- Files stay well under 1k lines; no sprawl problem (`undoredocommand.cpp` ~388, `undoredomanager.cpp` ~753).
- No new spaghetti in unrelated modules; the damage is local to undo/redo.

### 中文

- 把「`push` 觸發的自動 redo」放在 manager 上，而不是每個 `QUndoCommand` 自己記，這個模型是對的。
- `UndoRedoCommand::isFirstRedo()` 當轉發 helper，在 NVI 刪掉它之前，作為隔離層可以接受。
- 檔案都遠低於 1000 行，沒有膨脹問題（`undoredocommand.cpp` 約 388 行，`undoredomanager.cpp` 約 753 行）。
- 沒有把特殊分支散進無關模組；影響範圍只在 undo/redo。

---

## Required before merge / 合併前必須做的事

1. **NVI on `UndoRedoCommand`.** One `redo()` / `undo()` in the base; derived classes implement `redoImpl()` / `undoImpl()`. Delete the six `if (isFirstRedo())` branches.
2. **Name and hide the flag.** Do not ship `mFirstUndoInProgress` or public `isFirstRedoInProgress()`.
3. **RAII around `pushCommand`.** Restore the flag on every exit path (`QScopedValueRollback` or equivalent).
4. **A parent+child undo/redo test** for bitmap/vector replace + `TransformCommand`.

Until (1) is done, this is a flag relocation that still requires every command to cooperate. That is the design that already failed once.

---

1. **`UndoRedoCommand` 改成 NVI。** 基底類只留一個 `redo()` / `undo()`；衍生類實作 `redoImpl()` / `undoImpl()`。刪掉六個 `if (isFirstRedo())`。
2. **旗標改名並藏起來。** 不要把 `mFirstUndoInProgress` 或 public 的 `isFirstRedoInProgress()` 送上去。
3. **`pushCommand` 用 RAII 管旗標。** 任何離開路徑都要還原（`QScopedValueRollback` 或同等寫法）。
4. **補 parent+child 的 undo/redo 測試**，覆蓋點陣/向量取代 + `TransformCommand`。

第 1 點沒做完之前，這只是旗標搬家，仍然要求每個 command 自己配合。這套設計已經失敗過一次。
