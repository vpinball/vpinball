package org.vpinball.app

import android.net.Uri

data class VPinballUiState(
    val splash: Boolean = true,
    val loading: Boolean = false,
    val table: Table? = null,
    val title: String? = null,
    val progress: Int = 0,
    val status: String? = null,
    val error: String? = null,
    val playing: Boolean = false,
    val importUri: Uri? = null,
)
