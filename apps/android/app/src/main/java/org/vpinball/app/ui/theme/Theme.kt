package org.vpinball.app.ui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat
import org.vpinball.app.AppAppearance
import org.vpinball.app.VPinballManager
import org.vpinball.app.jni.VPinballSettingsSection.STANDALONE

object AppearanceState {
    var appearance by mutableStateOf(AppAppearance.DARK)
        private set

    fun load() {
        appearance = AppAppearance.fromInt(VPinballManager.loadValue(STANDALONE, "Appearance", AppAppearance.DARK.value))
    }

    fun set(value: AppAppearance) {
        appearance = value
        VPinballManager.saveValue(STANDALONE, "Appearance", value.value)
    }
}

private val DarkColorScheme =
    darkColorScheme(
        primary = Color.VpxDarkYellow,
        onPrimary = Color.Black,
        background = Color.LightBlack,
        onBackground = Color.White,
        surface = Color.LightBlack,
        onSurface = Color.White,
        surfaceContainer = Color(0xFF1C1C1E),
        surfaceContainerHigh = Color(0xFF2C2C2E),
        surfaceContainerHighest = Color(0xFF3A3A3C),
        onSurfaceVariant = Color(0xFFA1A1A6),
    )

private val LightColorScheme =
    lightColorScheme(
        primary = Color.VpxAccentLight,
        onPrimary = Color.White,
        background = Color.SystemGroupedBackground,
        onBackground = Color.Black,
        surface = Color.SystemGroupedBackground,
        onSurface = Color.Black,
        surfaceContainer = Color.White,
        surfaceContainerHigh = Color(0xFFF2F2F7),
        surfaceContainerHighest = Color(0xFFE5E5EA),
        onSurfaceVariant = Color(0xFF6E6E73),
    )

@Composable
fun isDarkAppearance(): Boolean =
    when (AppearanceState.appearance) {
        AppAppearance.DARK -> true
        AppAppearance.LIGHT -> false
        AppAppearance.SYSTEM -> isSystemInDarkTheme()
    }

@Composable
fun VPinballTheme(darkTheme: Boolean = isDarkAppearance(), content: @Composable () -> Unit) {
    val colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            (view.context as? Activity)?.window?.let { window ->
                val controller = WindowCompat.getInsetsController(window, view)
                controller.isAppearanceLightStatusBars = !darkTheme
                controller.isAppearanceLightNavigationBars = !darkTheme
            }
        }
    }

    MaterialTheme(colorScheme = colorScheme, typography = Typography, content = content)
}
