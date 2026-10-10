package org.vpinball.app.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

val Color.Companion.DarkBlack: Color
    get() = Color(0xFF0C0C0C)

val Color.Companion.LightBlack: Color
    get() = Color(0xFF101010)

val Color.Companion.SystemGroupedBackground: Color
    get() = Color(0xFFF2F2F7)

val Color.Companion.VpxRed: Color
    get() = Color(0xFFFD251D)

val Color.Companion.VpxDarkYellow: Color
    get() = Color(0xFFFEF716)

val Color.Companion.VpxAccentLight: Color
    get() = Color(0xFFC8161C)

val subtitleColor: Color
    @Composable get() = MaterialTheme.colorScheme.onBackground.copy(alpha = 0.7f)
