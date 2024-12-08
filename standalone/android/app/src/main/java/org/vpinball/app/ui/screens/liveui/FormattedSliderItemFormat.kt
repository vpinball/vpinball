package org.vpinball.app.ui.screens.liveui

import org.vpinball.app.jni.VPinballUnitConverter

enum class FormattedSliderItemFormat {
    DECIMAL,
    DEGREES,
    PERCENTAGE,
    SCALED_PERCENTAGE,
    INT_PERCENTAGE,
    CM,
    VPU_CM;

    fun formatValue(value: Float): String {
        return when (this) {
            DECIMAL -> "%.1f".format(value)
            DEGREES -> "%.1f °".format(value)
            PERCENTAGE -> "%.1f %%".format(value)
            SCALED_PERCENTAGE -> "%.1f %%".format(value * 100f)
            INT_PERCENTAGE -> "%d %%".format(value.toInt())
            CM -> "%.1f cm".format(value)
            VPU_CM -> "%.1f cm".format(VPinballUnitConverter.vpuToCM(value))
        }
    }
}
