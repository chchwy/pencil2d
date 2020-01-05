#ifndef BRUSHSETTING_H
#define BRUSHSETTING_H

#include <QList>

/// This enum should be interchangeable with MyPaintBrushSetting
enum class BrushSettingType {
        BRUSH_SETTING_OPAQUE,
        BRUSH_SETTING_OPAQUE_MULTIPLY,
        BRUSH_SETTING_OPAQUE_LINEARIZE,
        BRUSH_SETTING_RADIUS_LOGARITHMIC,
        BRUSH_SETTING_HARDNESS,
        BRUSH_SETTING_ANTI_ALIASING,
        BRUSH_SETTING_DABS_PER_BASIC_RADIUS,
        BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS,
        BRUSH_SETTING_DABS_PER_SECOND,
        BRUSH_SETTING_GRIDMAP_SCALE,
        BRUSH_SETTING_GRIDMAP_SCALE_X,
        BRUSH_SETTING_GRIDMAP_SCALE_Y,
        BRUSH_SETTING_RADIUS_BY_RANDOM,
        BRUSH_SETTING_SPEED1_SLOWNESS,
        BRUSH_SETTING_SPEED2_SLOWNESS,
        BRUSH_SETTING_SPEED1_GAMMA,
        BRUSH_SETTING_SPEED2_GAMMA,
        BRUSH_SETTING_OFFSET_BY_RANDOM,
        BRUSH_SETTING_OFFSET_Y,
        BRUSH_SETTING_OFFSET_X,
        BRUSH_SETTING_OFFSET_ANGLE,
        BRUSH_SETTING_OFFSET_ANGLE_ASC,
        BRUSH_SETTING_OFFSET_ANGLE_2,
        BRUSH_SETTING_OFFSET_ANGLE_2_ASC,
        BRUSH_SETTING_OFFSET_ANGLE_ADJ,
        BRUSH_SETTING_OFFSET_MULTIPLIER,
        BRUSH_SETTING_OFFSET_BY_SPEED,
        BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS,
        BRUSH_SETTING_SLOW_TRACKING,
        BRUSH_SETTING_SLOW_TRACKING_PER_DAB,
        BRUSH_SETTING_TRACKING_NOISE,
        BRUSH_SETTING_COLOR_H,
        BRUSH_SETTING_COLOR_S,
        BRUSH_SETTING_COLOR_V,
        BRUSH_SETTING_RESTORE_COLOR,
        BRUSH_SETTING_CHANGE_COLOR_H,
        BRUSH_SETTING_CHANGE_COLOR_L,
        BRUSH_SETTING_CHANGE_COLOR_HSL_S,
        BRUSH_SETTING_CHANGE_COLOR_V,
        BRUSH_SETTING_CHANGE_COLOR_HSV_S,
        BRUSH_SETTING_SMUDGE,
        BRUSH_SETTING_SMUDGE_LENGTH,
        BRUSH_SETTING_SMUDGE_RADIUS_LOG,
        BRUSH_SETTING_ERASER,
        BRUSH_SETTING_STROKE_THRESHOLD,
        BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC,
        BRUSH_SETTING_STROKE_HOLDTIME,
        BRUSH_SETTING_CUSTOM_INPUT,
        BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS,
        BRUSH_SETTING_ELLIPTICAL_DAB_RATIO,
        BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE,
        BRUSH_SETTING_DIRECTION_FILTER,
        BRUSH_SETTING_LOCK_ALPHA,
        BRUSH_SETTING_COLORIZE,
        BRUSH_SETTING_SNAP_TO_PIXEL,
        BRUSH_SETTING_PRESSURE_GAIN_LOG,
        BRUSH_SETTINGS_COUNT
};

struct BrushSettingInfo {
    QString cname;
    QString name;
    bool isConstant;
    float min;
    float defaultValue;
    float max;
    QString tooltip;
};

const QList<BrushSettingType> allSettings = { BrushSettingType::BRUSH_SETTING_OPAQUE,
                                              BrushSettingType::BRUSH_SETTING_OPAQUE_MULTIPLY,
                                              BrushSettingType::BRUSH_SETTING_OPAQUE_LINEARIZE,
                                              BrushSettingType::BRUSH_SETTING_RADIUS_LOGARITHMIC,
                                              BrushSettingType::BRUSH_SETTING_HARDNESS,
                                              BrushSettingType::BRUSH_SETTING_ANTI_ALIASING,
                                              BrushSettingType::BRUSH_SETTING_DABS_PER_BASIC_RADIUS,
                                              BrushSettingType::BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS,
                                              BrushSettingType::BRUSH_SETTING_DABS_PER_SECOND,
                                              BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE,
                                              BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_X,
                                              BrushSettingType::BRUSH_SETTING_GRIDMAP_SCALE_Y,
                                              BrushSettingType::BRUSH_SETTING_RADIUS_BY_RANDOM,
                                              BrushSettingType::BRUSH_SETTING_SPEED1_SLOWNESS,
                                              BrushSettingType::BRUSH_SETTING_SPEED2_SLOWNESS,
                                              BrushSettingType::BRUSH_SETTING_SPEED1_GAMMA,
                                              BrushSettingType::BRUSH_SETTING_SPEED2_GAMMA,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_BY_RANDOM,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_Y,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_X,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ASC,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_2_ASC,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_ANGLE_ADJ,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_MULTIPLIER,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED,
                                              BrushSettingType::BRUSH_SETTING_OFFSET_BY_SPEED_SLOWNESS,
                                              BrushSettingType::BRUSH_SETTING_SLOW_TRACKING,
                                              BrushSettingType::BRUSH_SETTING_SLOW_TRACKING_PER_DAB,
                                              BrushSettingType::BRUSH_SETTING_TRACKING_NOISE,
                                              BrushSettingType::BRUSH_SETTING_COLOR_H,
                                              BrushSettingType::BRUSH_SETTING_COLOR_S,
                                              BrushSettingType::BRUSH_SETTING_COLOR_V,
                                              BrushSettingType::BRUSH_SETTING_RESTORE_COLOR,
                                              BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_H,
                                              BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_L,
                                              BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSL_S,
                                              BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_V,
                                              BrushSettingType::BRUSH_SETTING_CHANGE_COLOR_HSV_S,
                                              BrushSettingType::BRUSH_SETTING_SMUDGE,
                                              BrushSettingType::BRUSH_SETTING_SMUDGE_LENGTH,
                                              BrushSettingType::BRUSH_SETTING_SMUDGE_RADIUS_LOG,
                                              BrushSettingType::BRUSH_SETTING_ERASER,
                                              BrushSettingType::BRUSH_SETTING_STROKE_THRESHOLD,
                                              BrushSettingType::BRUSH_SETTING_STROKE_DURATION_LOGARITHMIC,
                                              BrushSettingType::BRUSH_SETTING_STROKE_HOLDTIME,
                                              BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT,
                                              BrushSettingType::BRUSH_SETTING_CUSTOM_INPUT_SLOWNESS,
                                              BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_RATIO,
                                              BrushSettingType::BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE,
                                              BrushSettingType::BRUSH_SETTING_DIRECTION_FILTER,
                                              BrushSettingType::BRUSH_SETTING_LOCK_ALPHA,
                                              BrushSettingType::BRUSH_SETTING_COLORIZE,
                                              BrushSettingType::BRUSH_SETTING_SNAP_TO_PIXEL,
                                              BrushSettingType::BRUSH_SETTING_PRESSURE_GAIN_LOG
                                            };
#endif // BRUSHSETTING_H
