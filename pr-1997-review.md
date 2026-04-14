# PR #1997 Review: [GUI] Color Slider Update and Fixes

**Author:** MrStevns (Oliver Stevns)
**Status:** OPEN | No reviews yet
**Scope:** +649 / -252 lines, 12 files changed
**SonarQube:** Quality gate passed, but 20 new issues flagged

## Summary

This PR reworks the color slider and color preview widgets in the color inspector panel. It improves HiDPI rendering, fixes visual clipping artifacts, introduces pixmap caching for performance, centralizes picker position math, and adds a checkerboard backdrop for the color preview. It also introduces two new utility modules (`SliderPainter`/`SliderGeometry`) following a Data Oriented Design approach.

---

## Critical Issues

### 1. CMake build files not updated

New source files are added to `.pro` (qmake) only. The CMake build system (`app/app.cmake`, `core_lib/core_lib.cmake`) is not updated. This **will break CMake builds**.

Missing from `app/app.cmake`:
- `app/src/colorpreviewwidget.h`
- `app/src/colorpreviewwidget.cpp`

Missing from `core_lib/core_lib.cmake`:
- `core_lib/src/util/drawsliderstyle.h`
- `core_lib/src/util/drawsliderstyle.cpp`
- `core_lib/src/util/slidergeometry.h`
- `core_lib/src/util/slidergeometry.cpp`

### 2. `ColorPreviewWidget` constructor ignores parent parameter

In `colorpreviewwidget.cpp`:
```cpp
ColorPreviewWidget::ColorPreviewWidget(QWidget*)
{
}
```

The parent pointer is accepted but **never forwarded** to `QWidget(parent)`. This breaks Qt's parent-child ownership model, meaning the widget won't be properly cleaned up by the parent and could leak memory. Should be:
```cpp
ColorPreviewWidget::ColorPreviewWidget(QWidget* parent) : QWidget(parent)
{
}
```

---

## Bugs

### 3. Unused member `mColorPreviewPixmap`

`colorpreviewwidget.h` declares `QPixmap mColorPreviewPixmap;` but it is never read or written anywhere. Should be removed.

### 4. Typo: `ounterPen` instead of `outerPen`

In `colorslider.cpp`, the outer pen variable is named `ounterPen` (missing the leading "o"):
```cpp
QPen ounterPen;
ounterPen.setJoinStyle(Qt::MiterJoin);
...
```
Should be `outerPen`.

### 5. Duplicate checkerboard pixmap loading

In `colorslider.h`, the checkerboard pixmap is loaded twice:
```cpp
SliderPainterStyle mSliderStyle = SliderPainterStyle(
    QPalette::Dark,
    true,
    QBrush(QPixmap(":icons/general/checkerboard_smaller.png"))  // load #1
);

QPixmap mCheckerboardPixmap = QPixmap(":icons/general/checkerboard_smaller.png");  // load #2
```

Both are loaded per-instance at construction. The `mCheckerboardPixmap` is used to reassign `mSliderStyle.customFill` every paint cycle in `drawColorBox()`. Consider loading once and sharing.

---

## Design Concerns

### 6. Default tab changed from HSV to RGB

In `colorinspector.ui`, `currentIndex` changed from `0` to `1`, making the default visible tab RGB instead of HSV. If intentional, the PR description should mention this behavior change. If unintentional (e.g., Qt Designer artifact), it should be reverted.

### 7. Mutable cache fields mixed into config struct

`SliderPainterStyle` bundles configuration (roles, border width, radius ratio) with mutable cache state (`cachedSize`, `cachedCornerRadiusX/Y`). The struct is passed by non-const reference to paint functions that mutate the cache. This makes it unclear at call sites whether the struct is input-only or input+output. Consider splitting cache into a separate struct or using `mutable` fields.

### 8. `float` vs `qreal` type mixing

`SliderPainterStyle` uses `float` for `borderWidth`, `cornerRadiusRatio`, `cachedCornerRadiusX/Y`, but all QPainter APIs and `SliderGeometry` functions use `qreal` (double). This introduces implicit narrowing/widening conversions throughout the painting code. Should consistently use `qreal`.

### 9. Dead code branch in `updateSliderStyleCache`

```cpp
const qreal minRad = qMin(newSize.width(), newSize.height());
const qreal maxRad = qMax(newSize.width(), newSize.height());
qreal absolutePercentage = maxRad * radiusRatio;

if (minRad * radiusRatio < absolutePercentage) {
    style.cachedCornerRadiusX = minRad * radiusRatio;
} else {
    style.cachedCornerRadiusX = absolutePercentage;
}
```

The condition `minRad * radiusRatio < maxRad * radiusRatio` simplifies to `minRad < maxRad`, which is always true for a slider (width >> height). The `else` branch is dead code. The whole function could be:
```cpp
style.cachedCornerRadiusX = qMin(newSize.width(), newSize.height()) * style.cornerRadiusRatio;
style.cachedCornerRadiusY = style.cachedCornerRadiusX;
```

### 10. HSV conversion guard may leave color in wrong spec

In `colorinspector.cpp`:
```cpp
if (mCurrentColor.hue() != -1) {
    mCurrentColor = mCurrentColor.toHsv();
}
```

For achromatic colors (hue == -1), this skips the conversion entirely, leaving `mCurrentColor` in RGB spec while the HSV tab/sliders are active. This could cause `hsvHue()`, `hsvSaturation()`, and `value()` to return unexpected values when these sliders are updated.

---

## Minor / Style

### 11. `setColorSpec` naming

The method `setColorSpec()` doesn't set anything on the object -- it creates and returns a gradient. A name like `buildGradient()` or `createGradientForSpec()` would better convey intent. (Pre-existing, but this PR touches the surrounding code.)

### 12. `SliderPainterStyle` constructor inconsistency

The 6-arg constructor uses member-by-member assignment instead of initializer list. The 3-arg convenience constructor leaves `fillRole`, `borderWidth`, and `cornerRadiusRatio` to default-member-initializers, which is fine but could be confusing since `fillRole` defaults to `QPalette::Window` even though `hasCustomFill` is true.

---

## Known Issues (acknowledged by author)

- macOS: Spinboxes get clipped/squeezed despite `Preferred` layout policy. Not yet resolved.

---

## Verdict

**Not ready to merge.** The CMake build breakage (#1) and the missing parent forwarding (#2) must be fixed. The remaining bugs (#3-#5) and design issues (#6-#10) should be addressed or explicitly acknowledged before merging.

### Suggested review order
1. Fix critical issues #1 and #2
2. Fix bugs #3, #4, #5
3. Clarify or revert the default tab change (#6)
4. Address type mixing (#8) and dead code (#9)
5. Test HiDPI rendering and picker positioning after fixes
